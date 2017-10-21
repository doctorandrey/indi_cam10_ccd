# indi_cam10_ccd 

Making Indi driver for DIY Cam10 CMOS camera (Aptina/Micron MT9M001)

Original code was written in Delphi by Gilmanov Rim (Гильманов Рим).
Ascom driver was written by Sergiy Vakulenko.
Their code is shared on Sergiy Vakulenko's Github:
https://github.com/vakulenko/CAM10_software

>This port is based on the code of Gilles Le Maréchal (gehelem) - [github](https://github.com/gehelem/indi_cam86_ccd)

See:
- http://astroccd.org/category/cam10/
- http://www.astroclub.kiev.ua/forum/index.php?topic=28929.0
- http://indilib.org
- https://www.intra2net.com/en/developer/libftdi/documentation/
- http://www.fishcamp.com/pdf/mt9m001_1300_mono.pdf

Before starting:
>sudo cp 99-cam10.rules /etc/udev/rules.d/  
>sudo service udev restart  

To use it as stand alone application uncomment:  
>#define TESTING  

Then run it:  
>./indi_cam10_ccd -p 155 -e 0.5  

where -p is the hex value of the test pattern, -e exposure duration in seconds (min 0.1 max 1)  

_Andrei_ _Prakapovich_
