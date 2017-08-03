  <?php
/**
 * @file
 * Define functions to load all events.
 */

/**
 * Get all events for a given month.
 *
 * @param int $start
 *   Month to load
 *
 * @param int $end
 *   Timestamp of
 *
 * @return array
 *   Set of all events in a given month of the year.
 */
function kultur_calendar_get_events($start, $end) {
  $daily_events = array();

  $date = $start;
  while ($date <= $end) {
    $date = date('Y-m-d', strtotime('+1 day', $date));
    $daily_events[$date] = kultur_calendar_get_day_libraries_events($date);
    $date = strtotime($date);
  }

  return $daily_events;
}

  /**
   * Call to database to get grouped event amounts by library.
   *
   * @param string $date
   *   Date of the event
   *
   * @return array
   *   Set that contains all the results.
   */
function kultur_calendar_get_day_libraries_events($date) {
  $query = db_select('node', 'library');
  $query->leftJoin('og_membership','og', 'library.nid = og.gid AND og.entity_type = \'node\'');
  $query->leftJoin('node','event', 'event.type = \'ding_event\' AND og.etid = event.nid');
  $query->leftJoin('field_data_field_ding_event_date','event_date', 'event_date.entity_id = event.nid');
  $query->innerJoin('node','n', 'og.gid = n.nid AND n.type = \'ding_library\'');
  $query->addField('library', 'nid', 'lid');
  $query->addField('library', 'title', 'title');
  $query->addExpression('COUNT(event.nid)', 'amount');
  $query->condition('library.type', 'ding_library')
        ->condition('event.status', 1)
        ->where('DATE(event_date.field_ding_event_date_value) = :date', array('date' => $date));
  $query->groupBy('library.title');
  $result = $query->execute()->fetchAll();

  return $result;
}

  /**
   * Call to database to get single event for a given date and library/
   *
   * @param string $date
   *   Date of the event.
   * @param int $lid
   *   Library id.
   *
   * @return \stdClass
   *   Object that contains event info.
   */
function kultur_calendar_get_day_events($date, $lid) {
  $query = db_select('node', 'events');
  $query->leftJoin('field_data_field_ding_event_date', 'events_date', 'events.nid = events_date.entity_id');
  $query->leftJoin('og_membership', 'og', 'events.nid = og.etid');
  $query->addField('events', 'title', 'title');
  $query->addField('events', 'nid', 'eid');
  $query->addField('events_date', 'field_ding_event_date_value', 'start');
  $query->addField('events_date', 'field_ding_event_date_value2', 'end');
  $query->condition('events.type', 'ding_event')
        ->condition('og.gid', $lid)
        ->where('DATE(events_date.field_ding_event_date_value) = :date', array('date' => $date));
  $query->range(0, 1);
  $result = $query->execute()->fetchAll();

  return reset($result);
}
