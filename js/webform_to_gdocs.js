(function ($) {
  Drupal.behaviors.gsb_feature_admission_event_ct = {
    attach: function (context, settings) {
      // 2 > od4
      var gid_to_wid = function(gid) {
          var xorval = gid > 31578 ? 474 : 31578;
          var letter = gid > 31578 ? 'o' : '';
          return letter + parseInt((gid ^ xorval)).toString(36);
      }

      $(document).ready(function() {
        $("[name='gdoc_id']").blur(function() {
          var gdoc_id = $("[name='gdoc_id']").val();
          var gdoc_id = gdoc_id.split("#");
          var gid = gdoc_id[1].split("=");        
          $("[name='gdoc_sheet']").val(gid_to_wid(gid[1]));          
        });
      });
    }
  }
})(jQuery);