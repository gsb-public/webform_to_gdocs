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

  // Select active spreadsheet data if it exists.
  $result = db_query("SELECT * FROM {webform_to_gdocs_webforms} WHERE nid = :nid",array(':nid' =>  $node->nid));
  $gdoc_exists = $result->fetchAssoc();

  if ($gdoc_exists) {

    $form['gdoc_id']['#default_value'] = $gdoc_exists['gdoc_name'];
    $form['gdoc_type']['#default_value'] = $gdoc_exists['gdoc_type'];

    $form['gdoc_sheet'] = array(
      '#type' => 'hidden',
      '#title' => 'Worksheet ID',
     /* '#options' => array('_none' => '- None -'), */
    );

    $form['gdoc_sheet']['#default_value'] = $gdoc_exists['gdoc_sheet'];

  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' =>  'Save Settings',
  );

  return $form;
}

function webform_to_gdocs_attach_form_validate($form, &$form_state) {
 // if ($form_state['values']['gdoc_id'] != '_none' && empty($form_state['values']['gdoc_sheet'])) {
  //  form_set_error('gdoc_sheet', 'Enter a Worksheet Name to be used with the document you selected.');
 // }
}

function webform_to_gdocs_attach_form_submit($form, &$form_state) {
  $result = db_query("SELECT nid FROM {webform_to_gdocs_webforms} WHERE nid = :nid", array(':nid' => $form_state['values']['webform_nid']));
  $gdoc_exists = $result->fetchAssoc();
  if ($form_state['values']['gdoc_id'] == '_none') {
    // If a relationship already exists, delete it.
    if ($gdoc_exists) {
      db_query("DELETE FROM {webform_to_gdocs_webforms} WHERE nid = %d", $form_state['values']['webform_nid']);
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
