Install libftdi-dev and Git :

sudo apt-get install libftdi-dev git


Download the code :

git clone https://github.com/gehelem/indi_cam84xxx_ccd.git


Install udev rules :

cd indi_cam10_ccd
sudo cp 99-cam10.rules /etc/udev/rules.d/
sudo service udev restart


Build the driver :

mkdir build
cd build
cmake ..
make


Run the driver :

indiserver -v -m 100 ./indi_cam10_ccd
