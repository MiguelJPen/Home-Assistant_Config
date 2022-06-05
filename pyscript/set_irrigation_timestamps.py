from datetime import datetime as dt
from datetime import timedelta as tmd
from datetime import timezone as timezone

@service
def set_irrigation_timestamps():
    time_per_day = 15
    days_watering = 0
    day = 3600 * 24
    today = dt.now().replace(hour=1, minute=15)

    # For setting the next run of the script
    next_run = today + tmd(days = 5)
    next_run_aux = next_run.timestamp()
    input_datetime.set_datetime(
        entity_id = "input_datetime.irrigation_script_execution",
        timestamp = int(next_run_aux)
    )


    mean_temp = float(sensor.exterior_temperature_mean_over_last_72_hours)
    mean_humid = float(sensor.exterior_humidity_mean_over_last_72_hours)
    min_humid = float(sensor.exterior_humidity_minimum_over_last_72_hours)
    precip = float(sensor.precipitation)

    # First we calculate how much time per day
    if (mean_temp > 15 and mean_temp < 20):
        time_per_day += 15
    elif (mean_temp >= 20 and mean_temp < 25):
        time_per_day += 30
    elif (mean_temp >= 25):
        time_per_day += 45

    if (min_humid < 25 or mean_humid < 50):
        time_per_day += 20
    elif (min_humid < 45):
        time_per_day += 15

    # Now we calculate how many days from the next five
    if (precip >= 25):
        if (mean_temp <= 20):
            days_watering = 0
        else: days_watering = 1
    elif (precip >= 7):
        if (mean_temp <= 13):
            days_watering = 0
        elif (mean_temp > 13 and mean_temp <= 20):
            days_watering = 1
        else: days_watering = 2
    # QUEDA CUANDO NO HAY PRECIPITACION