<?php

function campaignmonitor_admin_settings_stats() {
  // Load list informations.
  $cm = CampaignMonitor::getConnector();
  $lists = $cm->getLists();

  // Loop over the lists and display stats for each list.
  $output = '';
  foreach ($lists as $id => $list) {
    $list_options = variable_get('campaignmonitor_list_' . $id, FALSE);
    $list_enabled = isset($list_options['status']['enabled']) ? $list_options['status']['enabled'] : 1;
    if ($list_enabled) {
      $output .= theme('campaignmonitor_list_stats', array('list' => $list, 'stats' => $cm->getListStats($id)));
    }
  }

  drupal_add_css(drupal_get_path('module', 'campaignmonitor') . '/css/campaignmonitor.admin.css');

  return $output;
}
