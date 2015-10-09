<?php

function webform_to_gdocs_attach_form($form_state, $nodedata) {

  $node=$nodedata['build_info']['args'][0];

  $form['webform_nid'] = array(
    '#type' => 'hidden',
    '#value' => $node->nid,
  );

  $form['gdoc_type'] = array(
    '#type' => 'hidden',
    '#default_value' => t('spreadsheet'),
  );

  $form['#attached']['js'] = array(
    array(
      'type' => 'file',
      'data' => drupal_get_path('module', 'webform_to_gdocs') . '/js/webform_to_gdocs.js',
    ),
  );

  $form['gdoc_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Google Spreadsheet URL'),
    '#description' => t('Enter the Google Spreadsheet URL. Example: https://docs.google.com/spreadsheets/d/1QEPF0ZtaTEP7MFt2SwAVHyTBS2akt6-YSyOIwbRIdg4/edit#gid=2049726158'),
    '#size' => 100,
    //'#options' => array('_none' => '- None -') + $spreadsheets,
  );

  $form['gdoc_sheet'] = array(
    '#type' => 'hidden',
  );

  // Select active spreadsheet data if it exists.
  $result = db_query("SELECT * FROM {webform_to_gdocs_webforms} WHERE nid = :nid",array(':nid' =>  $node->nid));
  $gdoc_exists = $result->fetchAssoc();

  if ($gdoc_exists) {
    $form['gdoc_id']['#default_value'] = $gdoc_exists['gdoc_name'];
    $form['gdoc_type']['#default_value'] = $gdoc_exists['gdoc_type'];
    $form['gdoc_sheet']['#default_value'] = $gdoc_exists['gdoc_sheet'];
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' =>  'Save Settings',
  );

  return $form;
}

function webform_to_gdocs_attach_form_validate($form, &$form_state) {
  // Check if spreadsheet is not empty and is valid
  if (!empty($form['gdoc_id']['#value'])) {
    if (!_webform_to_gdocs_connect_worksheet($form_state['values']['gdoc_id'])) {
      form_set_error('Google Spreadsheet URL', 'This is not valid Google spreadsheet, please enter valid Google spreadsheet URL.');
    }
  }
}

function webform_to_gdocs_attach_form_submit($form, &$form_state) {
  $result = db_query("SELECT nid FROM {webform_to_gdocs_webforms} WHERE nid = :nid", array(':nid' => $form_state['values']['webform_nid']));
  $gdoc_exists = $result->fetchAssoc();
  if (empty($form_state['values']['gdoc_id'])) {
    // If a relationship already exists, delete it.
     if ($gdoc_exists) {
      db_query("DELETE FROM {webform_to_gdocs_webforms} WHERE nid = :nid", array(':nid' => $form_state['values']['webform_nid']));
      drupal_set_message('Removed Google Doc attachment for this Webform.');
    }
    return;
  }

  // Insert record.
  if (!$gdoc_exists) {
    db_query("INSERT INTO {webform_to_gdocs_webforms} (nid, gdoc_type, gdoc_name, gdoc_sheet) VALUES (:nid, :type, :id, :sheet)",
         array(':nid' => $form_state['values']['webform_nid'], 
               ':type' => $form_state['values']['gdoc_type'], 
               ':id' => $form_state['values']['gdoc_id'], 
               ':sheet' => $form_state['values']['gdoc_sheet']));
  }

  // Update record.
  else {
    db_query("UPDATE {webform_to_gdocs_webforms} SET gdoc_type = :type, gdoc_name = :id, gdoc_sheet = :sheet WHERE nid = :nid", 
       array(':type' => $form_state['values']['gdoc_type'],
             ':id' =>  $form_state['values']['gdoc_id'], 
             ':sheet' => $form_state['values']['gdoc_sheet'], 
             ':nid' => $form_state['values']['webform_nid']));
  }

  drupal_set_message('Webform attached to Google Doc.');
}

/**
 * Check if the worksheet exists
 */
function _webform_to_gdocs_connect_worksheet($worksheet) {
  try {
    // Ensure we have a working access token.
    _webform_to_gdocs_google_drive_get_service();
    $access_token = variable_get('webform_to_gdocs_access_token', '');
    $access_token = @json_decode($access_token);

    // Construct the url for Google Spreadsheets API.
    $ch = curl_init();
    $url = 'https://spreadsheets.google.com/feeds/worksheets/' . _webform_to_gdocs_find_gdoc_name($worksheet) . '/private/full';

    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_HTTPGET, 1);
    //curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
    curl_setopt($ch, CURLOPT_HTTPHEADER, array(
        'Authorization: Bearer ' . $access_token->access_token,
        'Content-type: application/atom+xml'
    ));

    $result = curl_exec($ch);

    $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    if ($httpcode != 200) {
      //throw new Exception("Invalid response code from Google Docs API.");
      return false;
    }

    curl_close($ch);
    return true;
  }
  catch (Exception $e) {
    $msg = $e->getMessage();
    watchdog('webform_to_gdocs', $msg);
  }
}
