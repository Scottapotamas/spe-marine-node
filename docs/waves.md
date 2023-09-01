# Wave Height Monitoring Pipeline

These notes were internal thoughts/concepts during the project design phase, and have been kept for posterity.

___

Pressure sensor provides depth with `0.16 mBar` resolution when used in `8192` mode which would take ~16ms of ADC oversampling.

The actual design sample rate doesn't need to be this fast, but we might get better results by taking a faster stream of data and performing filtering manually.

## Test Data

I've not found any high-rate data for actual wave heights, only long-term observations in papers/online weather websites.

Testing the visualisations can likely be done with a series of superimposed signals:

- Low frequency sine to represent tidal changes
- Medium frequency, amplitude modulated sine to represent waves
- Some reasonably high frequency, low amplitude noise
- Gaussian/white noise + quantisation effects due to sensor behaviour and data processing

## Filtering

The raw signal should be filtered to retrieve as much of the original sine wave as possible, allowing for more accurate processing in later steps.

To start with, implementing the leaky integrator or a rolling median would probably work fine?

## Detecting waves

We want to know the height and wavelength/period.

To do this:

- Run a long-running average to provide baseline depth measurement.
- Perform min/max feature extraction to find the peaks and troughs of the waves.
  - Will likely need a debounce/timeout duration to handle noise
  - Ignore values within a deadband of the baseline depth, as 'very small' waves won't be reliable.
- With peak/trough event pairs, calculate the height of each wave.
- With peak event pairs, calculate the duration between them.
- Push height + period into a new datasource of waves

## Wave classification

With wave height and frequency data, we can then calculate some typical stats that meteorologists use/provide.

Information sourced from http://www.bom.gov.au/marine/knowledge-centre/reference/waves.shtml

We could attempt to classify waves into three types:

- Most frequent waves - about half the height of a significant wave
- Significant waves - about 14% of waves should be higher than these
- Maximum wave - about twice the height of significant waves. Expect 2-3 a day.

We could also render the sea-state value as per table:

| **Description** | **Height (metres)** | **Effect**                                                   | **WMO Sea State code** |
| --------------- | ------------------- | ------------------------------------------------------------ | ---------------------- |
| Calm (glassy)   | 0                   | No waves breaking on beach                                   | 0                      |
| Calm (rippled)  | 0 - 0.1             | No waves breaking on beach                                   | 1                      |
| Smooth          | 0.1 - 0.5           | Slight waves breaking on beach                               | 2                      |
| Slight          | 0.5 - 1.25          | Waves rock buoys and small craft                             | 3                      |
| Moderate        | 1.25 - 2.5          | Sea becoming furrowed                                        | 4                      |
| Rough           | 2.5 - 4             | Sea deeply furrowed                                          | 5                      |
| Very rough      | 4-6                 | Sea much disturbed with rollers having steep fronts          | 6                      |
| High            | 6-9                 | Sea much disturbed with rollers having steep fronts (damage to foreshore) | 7                      |

> Not sure what kind of time period to evaluate for sea-state values.
>
> Probably do a rolling X minute scan of the wave data and take the Yth percentile of heights?

## Visualisations

- Should attempt a realtime scrolling visualisation of wave height
  - Peak and trough detections annotated in real-time
  - Possibly horizontal/vertical annotations showing height and period?
  - Significant waves get a different colour or extra annotation?
  - Maximum waves get a different colour and annotation?

- Bar-chart comparing the height of each wave against the other as they are detected?
- Chart showing timing information for waves?
- Line-chart showing significant wave height and maximum wave height trends
  - Long-term data seems to be presented in like this https://www.qld.gov.au/environment/coasts-waterways/beach/monitoring/waves-sites/townsville

- Text display of sea-state code and description
- Text display of sea temperature

