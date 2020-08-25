# Reports

The combination of `sl` and the Stratify web application can create powerful reports from a variety of sources.

## Charts

A codeblock marked with `chartjs` will convert properly formed JSON into a chart using `chart.js`.  Here is a simple example:

```chartjs
{
  "type": "line",
  "options": {
    "scales": {
      "xAxes": [
        {
          "display": true,
          "type": "time",
          "distribution": "linear",
          "time": {
            "unit": "day",
            "displayFormats": {
              "day": "YYYY-MM-DD"
            }
          }
        }
      ],
      "yAxes": [
        {
          "display": true,
          "type": "logarithmic",
          "scaleLabel": {
            "display": true,
            "labelString": "Confirmed / Deaths (Log Scale)"
          }
        }
      ]
    },
    "legend": {
      "display": false
    },
    "title": {
      "display": true,
      "text": "COVID19 Cases (4 Day Logarithmic Moving Average)"
    }
  },
  "data": {
    "labels": [
      "2020-01-22",
      "2020-01-23",
      "2020-01-24",
      "2020-01-25",
      "2020-01-26",
      "2020-01-27",
      "2020-01-28",
      "2020-01-29",
      "2020-01-30",
      "2020-01-31"
    ],
    "datasets": [
      {
        "label": "Confirmed",
        "backgroundColor": "rgba(255,255,255,0.00)",
        "borderColor": "rgba(0,0,255,1.00)",
        "data": [
          555,
          653,
          783.8831787109375,
          959.5936889648438,
          1420.4462890625,
          2073.4931640625,
          3257.934326171875,
          4651.6962890625,
          6566.87451171875,
          7957.486328125
        ]
      },
      {
        "label": "Deaths",
        "backgroundColor": "rgba(255,255,255,0.00)",
        "borderColor": "rgba(0,0,0,1.00)",
        "data": [
          17,
          18,
          21.63330841064453,
          26.987651824951172,
          39.397647857666016,
          57.77638244628906,
          84.41592407226562,
          112.62777709960938,
          143.89305114746094,
          169.20338439941406
        ]
      }
    ]
  }
}
```

The chart above is created by marking the code block below as `chartjs`.

```json
{
  "type": "line",
  "options": {
    "scales": {
      "xAxes": [
        {
          "display": true,
          "type": "time",
          "distribution": "linear",
          "time": {
            "unit": "day",
            "displayFormats": {
              "day": "YYYY-MM-DD"
            }
          }
        }
      ],
      "yAxes": [
        {
          "display": true,
          "type": "logarithmic",
          "scaleLabel": {
            "display": true,
            "labelString": "Confirmed / Deaths (Log Scale)"
          }
        }
      ]
    },
    "legend": {
      "display": false
    },
    "title": {
      "display": true,
      "text": "COVID19 Cases (4 Day Logarithmic Moving Average)"
    }
  },
  "data": {
    "labels": [
      "2020-01-22",
      "2020-01-23",
      "2020-01-24",
      "2020-01-25",
      "2020-01-26",
      "2020-01-27",
      "2020-01-28",
      "2020-01-29",
      "2020-01-30",
      "2020-01-31"
    ],
    "datasets": [
      {
        "label": "Confirmed",
        "backgroundColor": "rgba(255,255,255,0.00)",
        "borderColor": "rgba(0,0,255,1.00)",
        "data": [
          555,
          653,
          783.8831787109375,
          959.5936889648438,
          1420.4462890625,
          2073.4931640625,
          3257.934326171875,
          4651.6962890625,
          6566.87451171875,
          7957.486328125
        ]
      },
      {
        "label": "Deaths",
        "backgroundColor": "rgba(255,255,255,0.00)",
        "borderColor": "rgba(0,0,0,1.00)",
        "data": [
          17,
          18,
          21.63330841064453,
          26.987651824951172,
          39.397647857666016,
          57.77638244628906,
          84.41592407226562,
          112.62777709960938,
          143.89305114746094,
          169.20338439941406
        ]
      }
    ]
  }
}
```