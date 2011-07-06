<?php

/*
 * @file
 * Implementation of the administration interface for the module.
 */


function campaignmonitor_admin_settings_general() {
  // Get account details.
  $account = variable_get('campaignmonitor_account', array());

  $form['campaignmonitor_account'] = array(
    '#type' => 'fieldset',
    '#title' => t('Account details'),
    '#description' => t('Account information from Campaign Monitor. See !link for more information.', array('!link' => l(t('documentation'), 'http://www.campaignmonitor.com/api/required/'))),
    '#collapsible' => empty($account) ? FALSE : TRUE,
    '#collapsed' => empty($account) ? FALSE : TRUE,
    '#tree' => TRUE,
  );

  $form['campaignmonitor_account']['api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#description' => t('Your Campaign Monitor API Key. See <a href="http://www.campaignmonitor.com/api/required/">documentation</a>.'),
    '#default_value' => isset($account['api_key']) ? $account['api_key'] : '',
    '#required' => TRUE,
  );

  $form['campaignmonitor_account']['client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Client ID'),
    '#description' => t('Your Campaign Monitor Client ID. See <a href="http://www.campaignmonitor.com/api/required/">documentation</a>.'),
    '#default_value' => isset($account['client_id']) ? $account['client_id'] : '',
    '#required' => TRUE,
  );

  if (!empty($account)) {
    $defaults = variable_get('campaignmonitor_general', array());
    $form['campaignmonitor_general'] = array(
      '#type' => 'fieldset',
      '#title' => t('General settings'),      
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#tree' => TRUE,
    );
 
    $form['campaignmonitor_general']['stats_cache_timeout'] = array(
      '#type' => 'textfield',
      '#title' => t('Statistics cache timeout'),
      '#description' => t('Cahce timeout in seconds for statistic information.'),
      '#size' => 4,
      '#default_value' => isset($defaults['stats_cache_timeout']) ? $defaults['stats_cache_timeout'] : '360',
    );


    $path = drupal_get_path('module', 'campaignmonitor') . '/lib/createsend-php/';
    $form['campaignmonitor_general']['library_path'] = array(
      '#type' => 'textfield',
      '#title' => t('Library path'),
      '#description' => t('Campign monitor library path.'),
      '#default_value' => isset($defaults['library_path']) ? $defaults['library_path'] : $path,
    );


    $form['clear_cache'] = array(
      '#type' => 'fieldset',
      '#title' => t('Clear cached data'),
      '#description' => t('This module uses cache to store information downloaded from campaign monitor. The information cached is lists datails, custom fields and more. To re-download the information, clear the cache.'),
    );

    $form['clear_cache']['clear'] = array(
      '#type' => 'submit',
      '#value' => t('Clear cached data'),
      '#submit' => array('campaignmonitor_clear_cache_submit'),
    );
  }
  
  return system_settings_form($form);
}

/**
 * Clear the caches used by campaign monitor.
 */
function campaignmonitor_clear_cache_submit($form, &$form_state) {
  CampaignMonitor::getConnector($form_state['values']['campaignmonitor_account']['api_key'], $form_state['values']['campaignmonitor_account']['client_id'])->clearCache();
  drupal_set_message(t('Campaign Monitor cache cleared.'), 'status');
}