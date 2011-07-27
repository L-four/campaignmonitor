<?php

function campaignmonitor_admin_settings_stats() {
  // Load list informations.
  $cm = CampaignMonitor::getConnector();
  $lists = $cm->getLists();

  // Loop over the lists and display stats for each list.
  $output = '';
  foreach ($lists as $id => $list) {
    if (campaignmonitor_is_list_enabled($id)) {
      // List was enable so get stats.
      $output .= theme('campaignmonitor_list_stats', array('list' => $list, 'stats' => $cm->getListStats($id)));
    }
  }

  drupal_add_css(drupal_get_path('module', 'campaignmonitor') . '/css/campaignmonitor.admin.css');
  return $output;
}
