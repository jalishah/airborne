
data = [0.0] * 100
count = 0
while True:
   try:
      val = int(file('/sys/class/hwmon/hwmon0/device/in7_input').read())
      #val /= 1024.0
      #val = (val + 27.0951) / 117.1319
      val = (val - 56.0) / 134.0
      data = data[1:] + [val]
      sum = 0.0
      for i in data:
         sum += i
      count += 1
      if count == 100:
         count = 0
         print sum / len(data)
   except KeyboardInterrupt:
      break
   except:
      pass
