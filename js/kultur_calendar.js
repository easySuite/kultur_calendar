(function($) {
  'use strict';

  Drupal.behaviors.kultur_calendar = {
    attach: function (context) {
      $('#kultur_calendar').fullCalendar({
        header: {
          left: '',
          center: 'prev title next',
          right: ''
        },
        firstDay: 1,
        events: function (start, end, timezone, callback) {
          $.ajax({
            url: `/kalender/events`,
            type: 'POST',
            dataType: 'json',
            data: {
              start: start.unix(),
              end: end.unix(),
            },
            success: function (doc) {
              let events = [];
              Object.keys(doc).forEach(function (date) {
                for (let value of doc[date]) {
                  let day = {
                    title: value.amount + ' ' + value.title,
                    url: `/node/${value.lid}`,
                    start: date,
                    lid: value.lid,
                  };
                  events.push(day);
                }
                // Add other link for pop-up.
                events.push({
                  start: date,
                  title: Drupal.t('See other'),
                  url: '/day',
                  date: date
                });
              });

              callback(events);
            }
          });
        },
        eventRender: function (event, element) {
          element.addClass('kultur-event');
          element.attr('data-lid', event.lid);
        },
        // Open pop-up on event click.
        eventClick: function (event, jsEvent, view) {
          if (event.url && event.date) {
            positionDayPopup(event);

            $.ajax({
              url: '/kalender/day',
              dataType: 'json',
              type: 'POST',
              data: {
                date: event.date
              },
              success: function (data) {
                let body = `<div class="kultur_calendar-body">
                  ${data[Object.keys(data)[0]].map(event =>
                    `<div class="row">
                      <div class="amount">${event.amount}</div>
                      <div class="title">${event.title}:</div>
                      <div class="event">
                        <a href="/node/${event.info.eid}">${event.info.title}</a>
                        <div class="time">
                          ${[event.info.start, event.info.end].filter(function(value) {return value;}).join(' - ')}
                        </div>
                      </div>
                    </div>`
                  ).join('')}
                </div>`;

                let $day = $('#kultur_calendar-day');
                let date = new Date(this.data.split('=')[1]);
                $day.find('.kultur_calendar-title').replaceWith(`
                    <div class="kultur_calendar-title">
                      ${Object.keys(data)[0]}
                      <div class="day-number">${date.getDate()}</div>
                    </div>
                  `);
                $day.find('.kultur_calendar-body').replaceWith(body);
              }
            });

            return false;
          }
        }
      });

      // Show/Hide events based on the selected library.
      $('#kultur-calendar', context).on('change', 'input:checkbox', function() {
        let lid = $(this).val();
        $(`a.fc-day-grid-event[data-lid='${lid}']`).parent().toggleClass('hidden');
      });

      $('#kultur-calendar').on('click', '#kultur_calendar-day', function() {
        $(this).toggleClass('hidden');
      });

      // Manipulate Day Popup on the calendar view.
      function positionDayPopup(event) {
        let day = $(".fc-bg").find(`td[data-date='${event.date}']`)[0];
        let left = ($(day).offset().left > $(day).width()) ? $(day).offset().left - $(day).width() : $(day).offset().left;
        let row = $(day).closest('.fc-row.fc-widget-content')[0];
        let nextRow = $(row).next()[0];
        let height = $(row).height();
        let top = 0;

        if (nextRow) {
          height += $(nextRow).height();
          top = $(nextRow).offset().top - $(row).height();
        } else {
          let prevRow = $(row).prev()[0];
          height += $(prevRow).height();

          top = $(prevRow).offset().top;
        }
        $('#kultur_calendar-day').css(
          {
            left: left,
            top: top,
            height: height
          }
        ).removeClass('hidden');

        // Prevent redirect.
        return false;
      }
    }
  };
} (jQuery));
