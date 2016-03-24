# TeelSys_THL
ATtiny85 Raspberry Pi Temperature Humidity Light Sensor

View the [GitHub Pages](http://teelsys.github.io/TeelSys_THL/) for this project.

More information regarding this project may be found on the [teelsys.com](http://teelsys.com) blog. Specific entries for this project include the following:

* [I2C Communications between Raspberry Pi and Arduino – Part One](http://teelsys.com/?p=244)
* [I2C Communications between Raspberry Pi and Arduino – Part Two](http://teelsys.com/?p=265)
* [I2C Communications between Raspberry Pi and Arduino – Part Three](http://teelsys.com/?p=314)
* [I2C Communications between Raspberry Pi and Arduino – Part Four](http://teelsys.com/?p=332)
* [I2C Communications between Raspberry Pi and Arduino – Update, BOM, and Source Control](http://teelsys.com/?p=393)

## Release Notes
### Version 0.1.0 - 21 March 2016
* Stable for up to three I2C devices
* Uses the PIGPIO library to bit bang the I2C bus (REF: http://abyz.co.uk/rpi/pigpio/pdif2.html)
* Documentation should be able to get anyone up and running with this version

#### TODO
* Writeup complete documentation from start to finish
  * Add documentation Raspberry Pi Hat (Not a true hat as there is no flash)
  * Add more detailed design documentation 

### Version 0.0.1 - 19 March 2016
* Stable for one I2C device only
* Uses the Raspberry Pi I2C hardware control