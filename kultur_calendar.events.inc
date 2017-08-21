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
  $daily_events = array();

  $did = kultur_calendar_get_domain_id();
  $timezone = date_default_timezone_get();

  // TODO: Request a task to make a single request and avoid loop.
  $date = $start;
  while ($date <= $end) {
    $date = date('Y-m-d', strtotime('+1 day', $date));
    $daily_events[$date] = kultur_calendar_get_day_libraries_events($date, $did, $timezone);
    $date = strtotime($date);
  }

  return $daily_events;
}

/**
 * Get kultur domain id.
 */
function kultur_calendar_get_domain_id() {
  $did = &drupal_static(__FUNCTION__);
  if (empty($did)) {
    $domains = domain_domains();
    $did = FALSE;
    foreach ($domains as $domain) {
      if (strpos($domain['machine_name'], 'kultur') !== FALSE) {
        $did = $domain['domain_id'];
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
 * @param int $did
 *   Domain grant access id.
 * @param string $timezone
 *   Timezone name.
 *
 * @return array
 *   Set that contains all the results.
 */
function kultur_calendar_get_day_libraries_events($date, $did, $timezone = 'Europe/Berlin') {
  $query = db_select('node', 'library');
  $query->leftJoin('og_membership', 'og', 'library.nid = og.gid AND og.entity_type = \'node\'');
  $query->leftJoin('node', 'event', 'event.type = \'ding_event\' AND og.etid = event.nid');
  $query->leftJoin('field_data_field_ding_event_date', 'event_date', 'event_date.entity_id = event.nid');
  $query->innerJoin('node', 'n', 'og.gid = n.nid AND n.type = \'ding_library\'');
  $query->addField('library', 'nid', 'lid');
  $query->addField('library', 'title', 'title');
  $query->addExpression('COUNT(event.nid)', 'amount');
  $query->condition('library.type', 'ding_library')
    ->condition('event.status', 1)
    ->where(':date >= DATE(CONVERT_TZ(field_ding_event_date_value, \'UTC\', :timezone))', array('date' => $date, ':timezone' => $timezone))
    ->where(':date <= DATE(CONVERT_TZ(field_ding_event_date_value2, \'UTC\', :timezone))', array('date' => $date, ':timezone' => $timezone));
  $query->groupBy('library.title');

  if ($did) {
    $query->leftJoin('domain_access', 'da', 'event.nid = da.nid');
    $query->condition('da.gid', $did);
  }

  $result = $query->execute()->fetchAll();

  return $result;
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
  $query->addExpression('CONVERT_TZ(events_date.field_ding_event_date_value, \'UTC\', :timezone)', 'start', array(':timezone' => $timezone));
  $query->addExpression('CONVERT_TZ(events_date.field_ding_event_date_value2, \'UTC\',  :timezone)', 'end', array(':timezone' => $timezone));
  $query->condition('events.type', 'ding_event')
    ->condition('og.gid', $lid)
    ->where(':date >= DATE(CONVERT_TZ(events_date.field_ding_event_date_value, \'UTC\', :timezone))', array('date' => $date, ':timezone' => $timezone))
    ->where(':date <= DATE(CONVERT_TZ(events_date.field_ding_event_date_value2, \'UTC\', :timezone))', array('date' => $date, ':timezone' => $timezone));
  $query->range(0, 1);

  if ($did) {
    $query->leftJoin('domain_access', 'da', 'events.nid = da.nid');
    $query->condition('da.gid', $did);
  }

  $result = $query->execute()->fetchAll();

  return reset($result);
}
