# Telemetry Contract

## Endpoint

`GET /api/telemetry`

The response is JSON and must identify whether values are valid or stale. The endpoint must not return the current hard-coded sample as if it were a measurement.

```json
{
  "schema_version": 1,
  "device_id": "robot_01",
  "state": {
    "mode": "MANUAL",
    "motion": "STOP",
    "speed": 0,
    "safety_reason": "NONE"
  },
  "sensors": {
    "temperature_c": 28.5,
    "humidity_percent": 65.0,
    "battery_v": 7.4,
    "distance_cm": 0,
    "line": [0, 0, 1, 0, 0],
    "validity": {
      "temperature": true,
      "humidity": true,
      "battery": true,
      "distance": true,
      "line": true
    }
  }
}
```

Unavailable or stale measurements must be represented by validity/error metadata, not silently replaced by zero. The line array length must match the configured sensor count.

## Sampling expectations

- Environment and battery values: refreshed every 2–5 seconds.
- Movement/safety state: updated on every accepted command and safety tick.
- UI may poll faster, but repeated reads must not cause a sensor read on every HTTP request.
