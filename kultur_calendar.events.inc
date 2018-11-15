<?php

/**
 * @file
 * Define functions to load all events.
 */

/**
 * Get all events for a given month.
 *
 * @param int $start
 *   Month to load.
 * @param int $end
 *   Timestamp of.
 *
 * @return array
 *   Set of all events in a given month of the year.
 */
function kultur_calendar_get_events($start, $end) {
  $daily_events = [];

  $did = kultur_calendar_get_domain_id();
  $timezone = date_default_timezone_get();

  $libraries_queue = nodequeue_load_queue_by_name('kultur_libraries');
  $libraries_sub_queue = nodequeue_load_subqueues_by_queue($libraries_queue->qid);
  $libraries_sub_queue = reset($libraries_sub_queue);
  $libraries = _kultur_calendar_get_libraries($libraries_sub_queue);

  $other_queue = nodequeue_load_queue_by_name('other_kultur_libraries');
  $other_sub_queue = nodequeue_load_subqueues_by_queue($other_queue->qid);
  $other_sub_queue = reset($other_sub_queue);
  $other_libraries = _kultur_calendar_get_libraries($other_sub_queue);

  // Loop through each day of the month because there can be some events which
  // last more then one day.
  $date = $start;
  while ($date <= $end) {
    $date = date('Y-m-d', strtotime('+1 day', $date));
    $daily_events[$date] = kultur_calendar_get_day_libraries_events($date, $did, $libraries, $libraries_sub_queue->sqid, $timezone);

    $daily_events[$date][] = kultur_calendar_get_day_other_libraries_events($date, $did, $other_libraries, $timezone);
    $date = strtotime($date);
  }

  return $daily_events;
}

/**
 * Get list of libraries ids based on sub-queue id.
 */
function _kultur_calendar_get_libraries($sub_queue) {
  $libraries = db_select('nodequeue_nodes', 'nn')
    ->fields('nn', ['nid'])
    ->condition('nn.sqid', $sub_queue->sqid)
    ->orderBy('nn.position', 'ASC')
    ->execute()
    ->fetchCol();

  return $libraries;
}

/**
 * Get kultur domain id.
 */
function kultur_calendar_get_domain_id() {
  $did = FALSE;
  if (module_exists('domain')) {
    $did = &drupal_static(__FUNCTION__);
    if (empty($did)) {
      $domains = domain_domains();
      $did = FALSE;
      foreach ($domains as $domain) {
        if (strpos($domain['machine_name'], 'kultur') !== FALSE) {
          $did = $domain['domain_id'];
        }
        else {
          $did = $domain['domain_id'];
        }
      }
    }
  }

  return $did;
}

/**
 * Call to database to get grouped event amounts by library.
 *
 * @param string $date
 *   Date of the event.
 * @param int|bool $did
 *   Domain grant access id.
 * @param array $libraries
 *   Set of libraries to be used for selection.
 * @param int $sqid
 *   Subqueue id, this parameter is needed for weight.
 * @param string $timezone
 *   Timezone name.
 *
 * @return array
 *   Set that contains all the results.
 */
function kultur_calendar_get_day_libraries_events($date, $did, $libraries, $sqid, $timezone = 'Europe/Berlin') {
  $query = db_select('node', 'library');
  $query->leftJoin('og_membership', 'og', 'library.nid = og.gid');
  $query->leftJoin('node', 'event', 'og.etid = event.nid');
  $query->leftJoin('field_data_field_ding_event_date', 'event_date', 'event_date.entity_id = event.nid');
  $query->innerJoin('node', 'n', 'og.gid = n.nid');
  $query->leftJoin('nodequeue_nodes', 'nn', 'library.nid = nn.nid');
  $query->addField('library', 'nid', 'lid');
  $query->addField('library', 'title', 'title');
  $query->addExpression('COUNT(event.nid)', 'amount');
  $query->addField('nn', 'position', 'weight');
  $query->condition('library.type', 'ding_library')
    ->condition('og.entity_type', 'node')
    ->condition('event.status', 1)
    ->condition('event.type', 'ding_event')
    ->condition('n.type', 'ding_library')
    ->condition('library.nid', $libraries, 'IN')
    ->condition('nn.sqid', $sqid)
    ->where(":date >= DATE(CONVERT_TZ(field_ding_event_date_value, 'UTC', :timezone))", [
      ':date' => $date,
      ':timezone' => $timezone,
    ]
    )
    ->where(":date <= DATE(CONVERT_TZ(field_ding_event_date_value2, 'UTC', :timezone))", [
      ':date' => $date,
      ':timezone' => $timezone,
    ]
    );
  $query->groupBy('library.title');

  if (!empty($did)) {
    $query->leftJoin('domain_access', 'da', 'event.nid = da.nid');
    $query->condition('da.gid', $did);
  }

  $result = $query->execute()->fetchAll();

  return $result;
}

/**
 * Get amount of events in the set of other libraries.
 *
 * @param string $date
 *   Date of the event.
 * @param int $did
 *   Domain grant access id.
 * @param array $libraries
 *   Set of libraries to be used for selection.
 * @param string $timezone
 *   Timezone name.
 *
 * @return \stdClass|bool
 *   Event object prepared for fullcalendar.
 */
function kultur_calendar_get_day_other_libraries_events($date, $did, $libraries, $timezone = 'Europe/Berlin') {
  $query = db_select('node', 'event');
  $query->leftJoin('field_data_field_ding_event_date', 'event_date', 'event_date.entity_id = event.nid');
  $query->leftJoin('og_membership', 'og', 'event.nid = og.etid');
  $query->addExpression('COUNT(event.nid)', 'amount');
  $query->condition('event.type', 'ding_event')
    ->condition('og.gid', $libraries, 'IN')
    ->condition('event.status', 1)
    ->where(":date >= DATE(CONVERT_TZ(field_ding_event_date_value, 'UTC', :timezone))", [
      ':date' => $date,
      ':timezone' => $timezone,
    ]
    )
    ->where(":date <= DATE(CONVERT_TZ(field_ding_event_date_value2, 'UTC', :timezone))", [
      ':date' => $date,
      ':timezone' => $timezone,
    ]
    );
  if ($did) {
    $query->leftJoin('domain_access', 'da', 'event.nid = da.nid');
    $query->condition('da.gid', $did);
  }
  try {
    $executed = $query->execute();
    $result = $executed->fetchAll();

    $amount = reset($result)->amount;
  }
  catch (Exception $e) {
    $amount = array();
  }

  if ($amount) {
    $object = new stdClass();
    $object->title = t('Other actors');
    $object->lid = 'other';
    $object->amount = $amount;
    // See other link has weight 9.
    $object->weight = 8;
    $object->libraries = $libraries;

    return $object;
  }

  return FALSE;
}

/**
 * Call to database to get single event for a given date and library.
 *
 * @param string $date
 *   Date of the event.
 * @param int $lid
 *   Library id.
 * @param int $did
 *   Domain grant access id.
 * @param string $timezone
 *   Timezone name.
 *
 * @return \stdClass
 *   Object that contains event info.
 */
function kultur_calendar_get_day_events($date, $lid, $did, $timezone = 'Europe/Berlin') {
  $query = db_select('node', 'events');
  $query->leftJoin('field_data_field_ding_event_date', 'events_date', 'events.nid = events_date.entity_id');
  $query->leftJoin('og_membership', 'og', 'events.nid = og.etid');
  $query->addField('events', 'title', 'title');
  $query->addField('events', 'nid', 'eid');
  $query->addExpression("CONVERT_TZ(events_date.field_ding_event_date_value, 'UTC', :timezone)", 'start', [':timezone' => $timezone]);
  $query->addExpression("CONVERT_TZ(events_date.field_ding_event_date_value2, 'UTC',  :timezone)", 'end', [':timezone' => $timezone]);
  $query->condition('events.type', 'ding_event')
    ->condition('og.gid', $lid)
    ->condition('events.status', 1)
    ->where(":date >= DATE(CONVERT_TZ(events_date.field_ding_event_date_value, 'UTC', :timezone))", [
      ':date' => $date,
      ':timezone' => $timezone,
    ]
    )
    ->where(":date <= DATE(CONVERT_TZ(events_date.field_ding_event_date_value2, 'UTC', :timezone))", [
      ':date' => $date,
      ':timezone' => $timezone,
    ]
    );
  $query->range(0, 1);

  if ($did) {
    $query->leftJoin('domain_access', 'da', 'events.nid = da.nid');
    $query->condition('da.gid', $did);
  }

  $result = $query->execute()->fetchAll();

  return reset($result);
}
