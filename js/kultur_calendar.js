(function($) {
  'use strict';

  Drupal.behaviors.kultur_calendar = {
    attach: function (context) {
      $('#kultur_calendar', context).fullCalendar({
        locale: $('html').attr('lang'),
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
            type: 'GET',
            dataType: 'json',
            data: {
              start: start.unix(),
              end: end.unix(),
            },
            success: function (doc) {
              let events = [];
              Object.keys(doc).forEach(function (date) {
                for (let value of doc[date]) {
                  if (value.amount > 0) {
                    let url = (value.lid !== 'other') ? `/node/${value.lid}` : `/arrangementer-liste/${date}`;
                    let day = {
                      title: value.title,
                      url: url,
                      start: date,
                      lid: value.lid,
                      amount: value.amount,
                      weight: value.weight,
                    };
                    events.push(day);
                  }
                }
                // Add other link for pop-up.
                if (doc[date].length > 1) {
                  events.push({
                    start: date,
                    title: Drupal.t('See other'),
                    url: `/arrangementer-liste/${date}`,
                    date: date,
                    weight: 9, // This link should be last in the list of events.
                  });
                }
              });

              callback(events);
            }
          });
        },
        eventOrder: ["weight"],
        eventRender: function (event, element, view) {
          if (view.name === 'month' && event.lid) {
            // Hide all items before render.
            element.addClass('hidden');

            // Process title before render.
            let amount = `<span class="event-amount">${event.amount}</span>`;
            // Trim title.
            let cutTitleLength = event.title.indexOf('-') !== -1 ?
              event.title.indexOf('-') :
              event.title.length;
            let title = amount + event.title.substr(0, cutTitleLength);
            element.find('span.fc-title').html(title);
          }

          element.addClass('kultur-event');
          if ($('#kultur-libraries').find('input[type=checkbox][value=' + event.lid + ']').is(':checked')) {
            element.removeClass('hidden');
          }
          element.attr('data-lid', event.lid);
          element.attr('data-date', event.start._i);

          let today = new Date().setUTCHours(0, 0, 0, 0) / 1000;
          let eventDay = event.start.unix();
          if (today < eventDay) {
            element.addClass('kultur-future-event');
          }
          else if (today > eventDay) {
            element.addClass('kultur-past-event');
          }
          else {
            element.addClass('kultur-today-event');
          }

          if (!event.lid) {
            element.addClass('other-info');
          }
        },
      });
      // Open popup on on day hover.
      $('#kultur_calendar').on('mouseenter','.fc-day', function(e) {
        let date = $(this).data('date');
        if ($(".fc-content-skeleton").find(`.kultur-event[data-date='${date}']`).length > 0) {
          positionDayPopup(date);
          $.ajax({
            url: '/kalender/day',
            dataType: 'json',
            type: 'GET',
            data: {
              date: date
            },
            success: function (data) {
              if (data[Object.keys(data)[0]] && data[Object.keys(data)[0]].length > 0) {
                let $day = $('#kultur_calendar-day');
                let date = Object.keys(data)[0].split(' ')[1].split('.')[0];

                let body = `
                <div class="kultur_calendar-title">
                  ${Object.keys(data)[0]}
                  <div class="day-number">${date}</div>
                </div>
                <div class="kultur_calendar-body">
                ${data[Object.keys(data)[0]].map(event =>
                  `<div class="row">
                    <div class="amount">${event.amount}</div>
                    <div class="title">${(event.lid === 'other') ? `<a href="/arrangementer-liste/${event.date}" class="other">${event.title}</a>` : `${event.title}:`}</div>
                    ${(event.lid != 'other') ?
                    `<div class="event">
                        <a href="/node/${event.info.eid}">${trimAndShorten(event.info.title)}</a>
                        <div class="time">
                          ${[event.info.start, event.info.end].filter(function (value) {
                      return value;
                    }).join(' - ')}
                        </div>
                      </div>` : ``}
                  </div>`
                ).join('')}
                </div>`;
                $day.find('.loading').replaceWith(body);
              }
            }
          });
        }
      });
      $('#kultur_calendar-day').on('mouseleave', function() {
        $(this).addClass('hidden');
      });
      // Show/Hide events based on the selected library.
      $('#kultur-libraries', context).on('change', 'input:checkbox', function() {
        let lid = $(this).val();
        $(`a.fc-day-grid-event[data-lid='${lid}']`).toggleClass('hidden');
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

      // Manipulate the length of event title in case it is very long and breaks
      // layout.
      function trimAndShorten(string) {
        if (string.length >= 60) {
          return string.substr(0, 60) + "...";
        }
        else {
          return string;
        }
      }

      // Manipulate Day Popup on the calendar view.
      function positionDayPopup(date) {
        let day = $(".fc-bg").find(`td[data-date='${date}']`)[0];
        let row = $(day).closest('.fc-row.fc-widget-content')[0];
        let nextRow = $(row).next()[0];
        let height = $(row).height();
        let aditionalCorrections = getAditionalCorrections();

        let top = 0;
        let left = ($(day).offset().left - aditionalCorrections.left > $(day).width()) ?
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

        $('#kultur_calendar-day').html(
          `<div class="loading">
            <div class="f_circleG" id="frotateG_01"></div>
            <div class="f_circleG" id="frotateG_02"></div>
            <div class="f_circleG" id="frotateG_03"></div>
            <div class="f_circleG" id="frotateG_04"></div>
            <div class="f_circleG" id="frotateG_05"></div>
            <div class="f_circleG" id="frotateG_06"></div>
            <div class="f_circleG" id="frotateG_07"></div>
            <div class="f_circleG" id="frotateG_08"></div>
          </div>`);
        // Prevent redirect.
        return false;
      }
    }
  };
} (jQuery));
