<?php

class CampaignMonitor {

  const CampaignMonitorCLIENT = 'csrest_clients.php';
  const CampaignMonitorLIST = 'csrest_lists.php';
  const CampaignMonitorSubscribers = 'csrest_subscribers.php';
  private $libraryPath = '/lib/createsend-php/';


  protected $api_key;
  protected $client_id;


  private $logErrors = TRUE;
  private $errors = array();
  private $clientObj = NULL;

  private $lists = array();

  private static $instance;

  /**
   * Private class constructor, which prevents creation of this class directly,
   * use the static function CampaignMonitor::GetConnector().
   *
   * @param string $api_key
   * @param string $client_id
   * @param string $libraryPath optional
   */
  private function __construct($api_key, $client_id, $libraryPath = NULL) {
    $this->api_key = $api_key;
    $this->client_id = $client_id;
    if ($libraryPath) {
      $libraryPath = $libraryPath;
    }

    // Get client
    if (!$this->createClientObj()) {
      echo "PIS";
      exit(-1);
    }
  }

  private function addError($type, $message) {
    $this->errors[] = array(
      'type' => $type,
      'message' => $message,
    );
    if ($this->logErrors) {
      watchdog('campaignmonitor', $message, NULL, $type);
    }
  }

  private function libraryLoad($file) {
    if ($this->libraryExists()) {
      @require_once drupal_get_path('module', 'campaignmonitor') . $this->libraryPath . $file;
      return TRUE;
    }
    $this->addError(WATCHDOG_ERROR, t('Unable to load client library.'));
    return FALSE;
  }

  private function createClientObj() {
    if ($this->clientObj) {
      return TRUE;
    }

    if ($this->libraryLoad(self::CampaignMonitorCLIENT)) {
      $this->clientObj = new CS_REST_Clients($this->client_id, $this->api_key);
      return TRUE;
    }
    return FALSE;
  }

  private function createListObj($listId) {
    if ($this->libraryLoad(self::CampaignMonitorLIST)) {
      return new CS_REST_Lists($listId, $this->api_key);
    }
    return FALSE;
  }

  private function createSubscriberObj($listId) {
    if ($this->libraryLoad(self::CampaignMonitorSubscribers)) {
      return new CS_REST_Subscribers($listId, $this->api_key);
    }
    return FALSE;
  }

  /**
   * Implements a singleton pattern that returns a instance of this object.
   *
   * @param string $api_key
   * @param string $client_key
   * @param string $libraryPath
   * @return object CampaingMonitor 
   */
  public static function getConnector($api_key, $client_key, $libraryPath = NULL) {
    if (!isset(self::$instance)) {
      $class = __CLASS__;
      self::$instance = new $class($api_key, $client_key, $libraryPath);
    }
    return self::$instance;
  }

  public function libraryExists() {
    if (file_exists(drupal_get_path('module', 'campaignmonitor') . $this->libraryPath)) {
      return TRUE;
    }
    return FALSE;
  }

  public function getErrors() {
    if (count($this->errors)) {
      return $this->errors;
    }
    return FALSE;
  }

  public function getLists() {
    if (empty($this->lists)) {
      if (($cache = cache_get('campaignmonitor_lists')) && !empty($cache->data)) {
        // Cache information found.
        $this->lists = $cache->data;
      }
      else {
        $result = $this->clientObj->get_lists();
        if ($result->was_successful()) {
          foreach ($result->response as $list) {
            $this->lists[$list->ListID] = array(
              'name' => $list->Name,
            );
          }
          cache_set('campaignmonitor_lists', $this->lists, 'cache');
        }
        else {
          $this->addError(WATCHDOG_ERROR, t('Failed with code: @code.', array('@code' => $result->http_status_code)));
        }
      }
    }

    return $this->lists;
  }

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
          $this->addError(WATCHDOG_ERROR, t('Failed with code: @code.', array('@code' => $result->http_status_code)));
        }
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
          $this->addError(WATCHDOG_ERROR, t('Failed with code: @code.', array('@code' => $result->http_status_code)));
        }
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

    // Load list details.
    if (!isset($this->lists[$listId]['details'])) {
      $this->getListDetails($listId);
    }

    // Load custom fields.
    if (!isset($this->lists[$listId]['CustomFields'])) {
      $this->getCustomFields($listId);
    }

    return $this->lists[$listId];
  }

  public function getSubscriber($listId, $email, $reset = FALSE) {
    static $subscriber;
    if (!isset($subscriber) || !isset($subscriber[$listId]) || $reset) {
      if (!isset($subscriber)) {
        $subscriber[] = array();
      }

      if ($obj = $this->createSubscriberObj($listId)) {
        $subscriber[$listId] = array();
        $result = $obj->get($email);
        if ($result->was_successful()) {
          foreach ($result->response as $key => $value) {
            $subscriber[$listId][$key] = $value;
          }
        }
        else {
          $this->addError(WATCHDOG_ERROR, t('Failed with code: @code.', array('@code' => $result->http_status_code)));
        }
      }
    }

    return $subscriber[$listId];
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
        $this->addError(WATCHDOG_ERROR, t('Failed with code: @code.', array('@code' => $result->http_status_code)));
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
        $this->addError(WATCHDOG_ERROR, t('Failed with code: @code.', array('@code' => $result->http_status_code)));
        return FALSE;
      }
      return TRUE;
    }
    return FALSE;
  }

  public function clearCache() {    
    cache_clear_all('campaignmonitor_lists', 'cache');
    cache_clear_all('campaignmonitor_list_detail', 'cache');
    cache_clear_all('campaignmonitor_custom_fields', 'cache');
  }
}