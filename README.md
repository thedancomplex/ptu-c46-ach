ptu-c46-ach
===========

PTU-c46 ach channel process


Required for installation
=========================

ACH
===

(1) Add this line to /etc/apt/sources.list:

    deb http://code.golems.org/debian squeeze golems.org

  (Replace squeeze with the codename for your distribution, ie lucid,
  precise, etc.)

(2) `sudo apt-get update`
(3) `sudo apt-get install libach1 libach-dev`
(4) `sudo apt-get install ach-utils`


Read Line
=========

sudo apt-get install libreadline-dev
