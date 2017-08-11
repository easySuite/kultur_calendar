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
        showNonCurrentDates: false,
        fixedWeekCount: false,
        columnFormat: 'dddd',
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

                  // Cut title bibliotec in the calendar.
                  let cutTitleLength = value.title.indexOf('-') !== -1 ?
                      value.title.indexOf('-') :
                      value.title.length;

                  let shortTitle = value.title.substr(0, cutTitleLength);

                  let day = {
                    title: value.amount + ' ' + shortTitle,
                    url: `/node/${value.lid}`,
                    start: date,
                    lid: value.lid,
                  };
                  // ----- end custom code

                  events.push(day);
                }
                // Add other link for pop-up.
                if (doc[date].length > 0) {
                  events.push({
                    start: date,
                    title: Drupal.t('See other'),
                    url: '/day',
                    date: date
                  });
                }
              });

              callback(events);
            }
          });
        },
        eventRender: function (event, element) {
          element.addClass('kultur-event');
          element.attr('data-lid', event.lid);

          let today = new Date().getDate();
          let eventDay = event.start.date();
          if (today < eventDay) {
            element.addClass('kultur-future-event');
          }
          else if (today === eventDay) {
            element.addClass('kultur-today-event');
          }
          else {
            element.addClass('kultur-past-event');
          }

          if (!event.lid) {
            element.addClass('other-info');
          }
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
      $('#kultur-libraries', context).on('change', 'input:checkbox', function() {
        let lid = $(this).val();
        $(`a.fc-day-grid-event[data-lid='${lid}']`).parent().toggleClass('hidden');
      });

      $('#kultur-calendar').on('click', '#kultur_calendar-day', function() {
        $(this).toggleClass('hidden');
      });

      // Move checkboxes in the calendar.
      $('#kultur-libraries').insertAfter(' #kultur_calendar .fc-toolbar');

      function getAditionalCorrections() {
        let result = {
          top: 0,
          left: 0
        };

        const parentPosition = $('.panel-pane.pane-page-content').css('position');
        if (!parentPosition || parentPosition === 'static' || parentPosition === 'inherit')  {
          return result;
        }
        return $('.panel-pane.pane-page-content').offset();
      }

      // Manipulate Day Popup on the calendar view.
      function positionDayPopup(event) {
        let day = $(".fc-bg").find(`td[data-date='${event.date}']`)[0];
        let row = $(day).closest('.fc-row.fc-widget-content')[0];
        let nextRow = $(row).next()[0];
        let height = $(row).height();
        let aditionalCorrections = getAditionalCorrections();

        let top = 0;
        let left = ($(day).offset().left > $(day).width()) ?
          $(day).offset().left - $(day).width() - aditionalCorrections.left :
          $(day).offset().left - aditionalCorrections.left;

        if (nextRow) {
          height += $(nextRow).height();
          top = $(nextRow).offset().top - $(row).height() - aditionalCorrections.top;
        } else {
          let prevRow = $(row).prev()[0];
          height += $(prevRow).height();

          top = $(prevRow).offset().top - aditionalCorrections.top;
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
