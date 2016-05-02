Description
===========
This data is a preprocessed version of Intel Lab Data, which can be accessed through: http://db.csail.mit.edu/labdata/labdata.html. There are 4 files: 

intelTemperatureTrain.csv
intelTemperatureTest.csv
intelHumidityTrain.csv
intelHumidity.csv

The files contain comma-separated sensor reading values. In each file, synchronized readings from 50 sensors are listed. Each row corresponds to a sensor, each column corresponds to a time stamp. They collected temperature and humidity data in an office environment. The time difference between each pair of readings is exactly 30 minutes. The first column shows sensor ids, and the first row shows time stamps in hour of day. E.g. 6.5 means 6:30 AM. In training files, there are 144 time stamps, whereas in test files, there are 96 time stamps. Note that the first time stamp of the test set follows immediately the last time stamp of the training set. Readings' time stamps are also synchronized across data. E.g. entry[4,56] of the temperature training data is the reading value of the 4th sensor at 56th time stamp. Entry[4,56] from the humidity training data is the humidity reading of the same sensor at the same time stamp.