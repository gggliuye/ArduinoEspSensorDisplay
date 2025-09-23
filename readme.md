
# ESP32 Sensor Display


```
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
export PATH=~/Arduino/bin:$PATH
```

```
./scripts/build_base.sh esp32 ttyUSB0 SensorDisplay
```


# ESP HOME

https://esphome.io/components/api


```
pip3 install esphome
mkdir ~/esphome
cd ~/esphome

esphome wizard livingroom_sensor.yaml
esphome run livingroom_sensor.yaml
```

```
esphome dashboard ~/esphome
```
