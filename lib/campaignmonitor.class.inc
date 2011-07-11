<?php

/**
 * @file
 * Implementation of the CampaignMonitor class, which is a wrapper class for
 * Campaign Monitor v3 API. It's implemented as a Singleton class and instances
 * are creted usign the account variables and the statuc function getConnector()
 * e.g.
 * 
 * $account = variable_get('campaignmonitor_account', array());
 * $cm = CampaignMonitor::getConnector($account['api_key'], $account['client_id']);
 *
 * If the class encounters any error in the communication with the Campaign
 * Monitor servers, then the last error can be extracted with getLatestError()
 * function e.g.
 *
 * if (!$cm->unsubscribe($listId, $email)) {
 *   $error = $cm->getLatestError();
 *   form_set_error('', $error['message']);
 * }
 *
 * This class is written by Jesper Kristensen (cableman@linuxdev.dk).
 */

class CampaignMonitor {

  // Used to load the different library parts of the API.
  const CampaignMonitorCLIENT = 'csrest_clients.php';
  const CampaignMonitorLIST = 'csrest_lists.php';
  const CampaignMonitorSubscribers = 'csrest_subscribers.php';
  private $libraryPath;


  protected $api_key;
  protected $client_id;


  private $logErrors = FALSE;
  private $errors = array();

  // These variables are used as static cache for the object.
  private $lists = array();
  private $listStats = array();
  private $campaigns = array();
  private $subscribers = array();

  // Holds the object instance (part of the singleton pattern).
  private static $instance;

  /**
   * Private class constructor, which prevents creation of this class directly,
   * use the static function CampaignMonitor::GetConnector().
   *
   * @param string $api_key
   * @param string $client_id
   * @param string $libraryPath optional
   */
  private function __construct($api_key, $client_id) {
    $this->api_key = $api_key;
    $this->client_id = $client_id;

    // Build library path.
    $setting = variable_get('campaignmonitor_general', array());
    if (isset($setting['library_path']) && !empty($setting['library_path'])) {
      $this->libraryPath = $setting['library_path'];
    }
    else {
      $this->libraryPath = drupal_get_path('module', 'campaignmonitor') . '/lib/createsend-php/';
    }

    // Enable logging ?.
    if (isset($setting['logging'])) {
      $this->logErrors = $setting['logging'];
    }
  }

  /**
   * Add an error the the local stack and call watchdog, if logging is enabled.
   *
   * @param string $type drupal watchdog const. error type
   * @param string $message
   * @param int $code normally the http response code
   */
  private function addError($type, $message, $code = -1) {
    $this->errors[] = array(
      'type' => $type,
      'code' => $code,
      'message' => t($message),
    );
    if ($this->logErrors) {
      $msg = t('Failed with code: @code and message: $msg', array('@code' => $code, '@msg' => $message));
      watchdog('campaignmonitor', $msg, NULL, $type);
    }
  }

  /**
   * Helper function that loads the part of the API defined in the $file
   * variable. This function is mostly used by create[xxx]Obj functions below.
   *
   * @param string $file the name of the API file to include
   * @return boolean TRUE of success else FALSE
   */
  private function libraryLoad($file) {
    if ($this->libraryExists()) {
      @require_once $this->libraryPath . $file;
      return TRUE;
    }
    $this->addError(WATCHDOG_ERROR, t('Unable to load client library.'));
    return FALSE;
  }

  /**
   * Create API client object.
   *
   * @return CS_REST_Clients campaign monitor client object or FALSE
   */
  private function createClientObj() {
    if ($this->libraryLoad(self::CampaignMonitorCLIENT)) {
      return new CS_REST_Clients($this->client_id, $this->api_key);
    }
    $this->addError(WATCHDOG_ERROR, t('Failed to locate the client library.'));
    return FALSE;
  }

  /**
   * Create API list object.
   *
   * @param string $listId campaign monitor list id
   * @return CS_REST_Lists campaign monitor list object or FALSE
   */
  private function createListObj($listId) {
    if ($this->libraryLoad(self::CampaignMonitorLIST)) {
      return new CS_REST_Lists($listId, $this->api_key);
    }
    $this->addError(WATCHDOG_ERROR, t('Failed to locate the list library.'));
    return FALSE;
  }

  /**
   * Create API subscribers object.
   *
   * @param type $listId campaign monitor list id
   * @return CS_REST_Subscribers campaign monitor subscriber object or FALSE
   */
  private function createSubscriberObj($listId) {
    if ($this->libraryLoad(self::CampaignMonitorSubscribers)) {
      return new CS_REST_Subscribers($listId, $this->api_key);
    }
    $this->addError(WATCHDOG_ERROR, t('Failed to locate the subscribe library.'));
    return FALSE;
  }

  /**
   * Create an unix timestamp based on the cache timeout set in the administration
   * interface.
   *
   * @return Unix timestamp
   */
  private function getCacheTimeout() {
    $options = variable_get('campaignmonitor_general', array());
    return time() + (isset($options['cache_timeout']) ? $options['cache_timeout'] : '360');
  }

  /**
   * Implements a singleton pattern that returns a instance of this object. The
   * function requires campaign monitor account keys to create the connection.
   * These keys can be found at the campaign monitor homepage and should be
   * entered in the administration interface and the object can be create like
   * below.
   *
   * $account = variable_get('campaignmonitor_account', array());
   * $cm = CampaignMonitor::getConnector($account['api_key'], $account['client_id']);
   *
   * @param string $api_key
   * @param string $client_key
   * @param string $libraryPath
   * @return object CampaingMonitor 
   */
  public static function getConnector($api_key, $client_key) {
    if (!isset(self::$instance)) {
      $class = __CLASS__;
      self::$instance = new $class($api_key, $client_key);
    }
    return self::$instance;
  }

  /**
   * Returns the lastest error from the stack of possible errors encountered 
   * during communication with campaign monitor servers.
   *
   * @return array with an error code and message.
   */
  public function getLatestError() {
    if (!empty($this->errors)) {
      $last = $this->errors[count($this->errors) - 1];
      return array(
        'code' => $last['code'],
        'message' => $last['message'],
      );
    }
    else {
      return array(
        'code' => 1,
        'message' => t('There dos not seam to been any errors.')
      );
    }
  }

  /**
   * Helper function that checks if the library path given exists.
   *
   * @return boolean if the library exists TRUE is retuerned else FALSE
   */
  public function libraryExists() {
    if (file_exists($this->libraryPath)) {
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Returns the internal error array with the format below.
   *
   * $errors[] = array(
   *   'type' => [watchdog error type],
   *   'code' => [error code],
   *   'message' => [message],
   * );
   *
   * @return array of errors or FALSE if the array is empty
   */
  public function getErrors() {
    if (count($this->errors)) {
      return $this->errors;
    }
    return FALSE;
  }

  /**
   * Reset the internal error array to the empty array.
   */
  public function resetErrors() {
    $this->errors = array();
  }

  /**
   * Gets all list from campaign monitor found under the client id given during
   * object creation. The list is returned as a keyed array and cached in the
   * cache table, so it may not always return the newest information.
   *
   * The array has the format below, be aware that the same cache is used be the
   * functions getListDetails and getCustomFields. This means that the information
   * returned by this function may contain extend list information, if any of
   * these functions have been called.
   *
   * $list[$id] = array(
   *  'name' => 'List name',
   * );
   *
   * @return array of lists avaliable from campaign monitor
   */
  public function getLists() {
    if (empty($this->lists)) {
      if (($cache = cache_get('campaignmonitor_lists')) && !empty($cache->data)) {
        // Cache information found.
        $this->lists = $cache->data;
      }
      else {
        // Create list object and get the lists, the lists are saved in the local
        // cache.
        if ($obj = $this->createClientObj()) {
          $result = $obj->get_lists();
          if ($result->was_successful()) {
            foreach ($result->response as $list) {
              $this->lists[$list->ListID] = array(
                'name' => $list->Name,
              );
            }
            cache_set('campaignmonitor_lists', $this->lists, 'cache');
          }
          else {
            $this->addError(WATCHDOG_ERROR, $result->response->Message, $result->http_status_code);
          }
        }
        else {
          return FALSE;
        }
      }
    }

    return $this->lists;
  }

  /**
   *
   * @param type $listId
   * @return type 
   */
  public function getListDetails($listId) {    
    // If lists have not been loaded yet, get them as they build the basic cache.
    if (empty($this->lists)) {
      $this->getLists();
    }

    // Test that the listId is valided.
    if (!isset($this->lists[$listId])) {
      $this->addError(WATCHDOG_ERROR, t('Unknown list id @listID.', array('@listID' => $listId)));
      return FALSE;
    }

    if (!isset($this->lists[$listId]['details'])) {
      if ($obj = $this->createListObj($listId)) {
        $result = $obj->get();
        if ($result->was_successful()) {
          // Save details about this list.
          $this->lists[$listId]['details'] = array();
          foreach ($result->response as $key => $value) {
            if (!in_array($key, array('ListID', 'Title'))) {
              $this->lists[$listId]['details'][$key] = $value;
            }
          }

          // Update cache with list details.
          cache_set('campaignmonitor_lists', $this->lists, 'cache');
        }
        else {
          $this->addError(WATCHDOG_ERROR, $result->response->Message, $result->http_status_code);
          return FALSE;
        }
      }
      else {        
        return FALSE;
      }
    }
    
    return $this->lists[$listId]['details'];
  }

  public function getCustomFields($listId) {
    // If lists have not been loaded yet, get them as they build the basic cache.
    if (empty($this->lists)) {
      $this->getLists();
    }

    // Test that the listId is valided.
    if (!isset($this->lists[$listId])) {
      $this->addError(WATCHDOG_ERROR, t('Unknown list id @listID.', array('@listID' => $listId)));
      return FALSE;
    }

    if (!isset($this->lists[$listId]['CustomFields'])) {
      if ($obj = $this->createListObj($listId)) {
        $result = $obj->get_custom_fields();
        if ($result->was_successful()) {
          // Save custom field about this list.
          $this->lists[$listId]['CustomFields'] = array();
          foreach ($result->response as $field) {
            foreach ($field as $name => $details) {
              $this->lists[$listId]['CustomFields'][$field->Key][$name] = $details;
            }
          }
          
          // Update cache with list details.
          cache_set('campaignmonitor_lists', $this->lists, 'cache');
        }
        else {
          $this->addError(WATCHDOG_ERROR, $result->response->Message, $result->http_status_code);
        }
      }
      else {
        return FALSE;
      }
    }
    
    return $this->lists[$listId]['CustomFields'];
  }

  public function getExtendedList($listId) {
    // If lists have not been loaded yet, get them as they build the basic cache.
    if (empty($this->lists)) {
      $this->getLists();
    }

    // Test that the listId is valided.
    if (!isset($this->lists[$listId])) {
      $this->addError(WATCHDOG_ERROR, t('Unknown list id @listID.', array('@listID' => $listId)));
      return FALSE;
    }

    // Load list details and custom fields (it's is_array has it may return an
    // empty array).
    if (!$this->getListDetails($listId) || !is_array($this->getCustomFields($listId))) {
      $this->addError(WATCHDOG_ERROR, t('Could not retrive extended information for @listID.', array('@listID' => $listId)));
      return FALSE;
    }

    return $this->lists[$listId];
  }

  /**
   * Update remote list information. The $options array should have the fields:
   * Title, UnsubscribePage, ConfirmedOptIn and ConfirmationSuccessPage.
   *
   * @param type $listId
   * @param type $options
   * @return type
   */
  public function updateList($listId, $options = array()) {
    // Make sure that list is loaded.
    if (!$this->getListDetails($listId)) {
      $this->addError(WATCHDOG_ERROR, t('Could not retrive update list information for @listID.', array('@listID' => $listId)));
      return FALSE;
    }

    // Get list object and update the list.
    if ($obj = $this->createListObj($listId)) {
      // @todo: check that the options are correct.
      $result = $obj->update($options);
      if($result->was_successful()) {

        // Update cache local list.
        $this->lists[$listId]['name'] = $options['Title'];
        $this->lists[$listId]['details']['UnsubscribePage'] = $options['UnsubscribePage'];
        $this->lists[$listId]['details']['ConfirmedOptIn'] = $options['ConfirmedOptIn'];
        $this->lists[$listId]['details']['ConfirmationSuccessPage'] = $options['ConfirmationSuccessPage'];

        // Update the cache.
        cache_set('campaignmonitor_lists', $this->lists, 'cache');
        return TRUE;
      } else {
        $this->addError(WATCHDOG_ERROR, $result->response->Message, $result->http_status_code);
      }
    }
    return FALSE;
  }

  public function getListStats($listId) {
    $fetch = FALSE;
    if (!isset($this->listStats[$listId])) {
      // Not found inside objec, try cache.
      if (($cache = cache_get('campaignmonitor_list_stats')) && !empty($cache->data)) {
        // Cache information found.
        $this->listStats = $cache->data;
        if (!isset($this->listStats[$listId])) {
          // Not found inside cache either.
          $fetch = TRUE;
        }
      }
      else {
        // No cache found or expired.
        $fetch = TRUE;
      }
    }

    if ($fetch) {
      if ($obj = $this->createListObj($listId)) {
        // Get stats from camapign monitor.
        $result = $obj->get_stats();
        if ($result->was_successful()) {
          $this->listStats[$listId] = (array)$result->response;

          // Update the cache.
          cache_set('campaignmonitor_list_stats', $this->listStats, 'cache', $this->getCacheTimeout());
        }
        else {
          $this->addError(WATCHDOG_ERROR, $result->response->Message, $result->http_status_code);
          return FALSE;
        }
      }
      else {
        return FALSE;
      }
    }

    return $this->listStats[$listId];
  }

  public function deleteList($listId) {
    if ($obj = $this->createListObj($listId)) {
      $result = $obj->delete();
      if ($result->was_successful()) {
        unset($this->lists[$listId]);
        cache_set('campaignmonitor_lists', $this->lists, 'cache');
        return TRUE;
      }
      else {
        $this->addError(WATCHDOG_ERROR, $result->response->Message, $result->http_status_code);
        return FALSE;
      }
    }
    return FALSE;
  }

  public function createList($client_id, $title, $unsubscribePage = '', $confirmedOptIn = FALSE, $confirmationSuccessPage = '') {
    if ($obj = $this->createListObj(NULL)) {
      $result = $obj->create($client_id, array(
        'Title' => check_plain($title),
        'UnsubscribePage' => check_plain($unsubscribePage),
        'ConfirmedOptIn' => $confirmedOptIn,
        'ConfirmationSuccessPage' => check_plain($confirmationSuccessPage),
      ));
      if ($result->was_successful()) {
        $this->clearCache();
        return TRUE;
      }
      else {
        $this->addError(WATCHDOG_ERROR, $result->response->Message, $result->http_status_code);
        return FALSE;
      }
    }
    return FALSE;
  }

  public function getCampaigns() {
    if (empty($this->campaigns)) {
      if (($cache = cache_get('campaignmonitor_campaigns')) && !empty($cache->data)) {
        // Cache information found.
        $this->campaigns = $cache->data;
      }
      else {
        if ($obj = $this->createClientObj()) {
          $result = $obj->get_campaigns();
          if ($result->was_successful()) {
            // Build an array foreach campaign returned.
            foreach ($result->response as $campaign) {
              $this->campaigns[$campaign->CampaignID] = array(
                'Name' => $campaign->Name,
                'Subject' => $campaign->Subject,
                'Sent' => strtotime($campaign->SentDate),
                'Recipients' => $campaign->TotalRecipients,
                'Link' => $campaign->WebVersionURL,
              );
            }
            // Save campaigns in the cache.
            cache_set('campaignmonitor_campaigns', $this->campaigns, 'cache', $this->getCacheTimeout());
          }
          else {
            $this->addError(WATCHDOG_ERROR, $result->response->Message, $result->http_status_code);
            return FALSE;
          }
        }
        else {
          return FALSE;
        }
      }
    }
    return $this->campaigns;
  }


  public function getSubscriber($listId, $email, $reset = FALSE) {
    $fetch = FALSE;
    if (!isset($this->subscribers[$listId . $email])) {
      // Not found inside objec, try cache.
      if (($cache = cache_get('campaignmonitor_subscribers')) && !empty($cache->data)) {
        // Cache information found.
        $this->subscribers = $cache->data;
        if (!isset($this->subscribers[$listId . $email])) {
          // Not found inside cache either.
          $fetch = TRUE;
        }
      }
      else {
        // No cache found or expired.
        $fetch = TRUE;
      }
    }

    if ($fetch) {
      if ($obj = $this->createSubscriberObj($listId)) {
        $this->subscribers[$listId . $email] = array();
        $result = $obj->get($email);
        if ($result->was_successful()) {
          foreach ($result->response as $key => $value) {
            if ($key == 'CustomFields') {
              // Make the custom fields object into a keyed array.
              $this->subscribers[$listId . $email][$key] = array();
              foreach ($value as $field) {
                $this->subscribers[$listId . $email][$key][$field->Key] = $field->Value;
              }
            }
            else {
              $this->subscribers[$listId . $email][$key] = $value;
            }
          }
          // Save the campaign information in the cache.
          cache_set('campaignmonitor_subscribers', $this->subscribers, 'cache', $this->getCacheTimeout());
        }
        else {
          $this->addError(WATCHDOG_ERROR, $result->response->Message, $result->http_status_code);
          return array();
        }
      }
      else {
        return FALSE;
      }
    }

    return $this->subscribers[$listId . $email];
  }

  public function isSubscribed($listId, $email, $reset = FALSE) {
    $result = $this->getSubscriber($listId, $email, $reset);
    if (!empty($result)) {
      if ($result['State'] == 'Active') {
        return TRUE;
      }
    }
    return FALSE;
  }

  public function subscribe($listId, $name, $email, $customFields = array()) {
    if ($obj = $this->createSubscriberObj($listId)) {
      $result = $obj->add(array(
          'EmailAddress' => $email,
          'Name' => $name,
          'CustomFields' => $customFields,
          'Resubscribe' => true,
      ));
      if (!$result->was_successful()) {
        $this->addError(WATCHDOG_ERROR, $result->response->Message, $result->http_status_code);
        return FALSE;
      }
      return TRUE;
    }
    return FALSE;
  }

  public function unsubscribe($listId, $email) {
    if ($obj = $this->createSubscriberObj($listId)) {
      $result = $obj->unsubscribe($email);
      if (!$result->was_successful()) {
        $this->addError(WATCHDOG_ERROR, $result->response->Message, $result->http_status_code);
        return FALSE;
      }
      return TRUE;
    }
    return FALSE;
  }

  public function clearCache() {    
    cache_clear_all('campaignmonitor_lists', 'cache');
    cache_clear_all('campaignmonitor_list_stats', 'cache');
    cache_clear_all('campaignmonitor_campaigns', 'cache');
    cache_clear_all('campaignmonitor_subscribers', 'cache');
  }
}