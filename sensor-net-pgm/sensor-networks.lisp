(defpackage :edu.sdimitro.sensor-networks
  (:use :common-lisp)
  (:import-from :split-sequence :split-sequence)
  (:import-from "PARSE-NUMBER" :parse-number))

(in-package :edu.sdimitro.sensor-networks)

(defparameter *data-index*
  '((humidity-train
     "intelLabDataProcessed/intelHumidityTrain.csv")
    (temperature-train
     "intelLabDataProcessed/intelTemperatureTrain.csv")
    (humidity-test
     "intelLabDataProcessed/intelHumidityTest.csv")
    (temperature-test
     "intelLabDataProcessed/intelTemperatureTest.csv"))
  "Train and test data from humidity and temperature sensors")

(defun get-datafile (category)
  "Return path of file containing data from this category"
  (cadr (assoc category *data-index*)))

(defun get-dimensions (filename)
  "Returns number of sensors and timestamps given a data file"
  (with-open-file (input filename)
    (let* ((header (read-line input nil))
	   (sensor-counter 0)
	   (reading-counter (count #\, (coerce header 'list))))
      (progn
	(loop for line = (read-line input nil)
	   while line do
	     (incf sensor-counter)))
      `(,sensor-counter ,reading-counter))))

(defstruct sensor
  id
  mean-hum
  mean-temp
  var-hum
  var-temp)

(defstruct sensor-net
  name
  sensors)

(defun generate-sensor-network (name size)
  (let ((sn (make-sensor-net
	     :name name
	     :sensors (make-array size :fill-pointer 0))))
    (dotimes (n size)
      (vector-push (make-sensor) (sensor-net-sensors sn)))
    sn))

(defvar *test-line*
  "0,3.65E+01,3.71E+01,3.73E+01")

(defun test ()
  (parse-number (cadr (split-sequence #\, *test-line*))))
