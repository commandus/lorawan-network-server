#include <stdio.h>
#include <string.h>
#include <usb.h>

#define DEBUG_LEVEL 0

struct usb_bus *busses,*bus,*dbus;

int main(int argc, char *argv[])
{
        struct usb_device *dev;
        char *buf;
        int n,x,r, ret;
        char *ibus, *idev;
        char string[64];
        int found = 0;       
        usb_dev_handle *udev;
        
        usb_set_debug(DEBUG_LEVEL);

        usb_init(); //initilize the usb library
        usb_find_busses();
        usb_find_devices();
        busses=usb_get_busses();

        //use the command line arguments for Bus and Device
        if (argc >= 3) {
                ibus = argv[1];
                idev = argv[2];
        }
        else { printf("Usage: usbtest <Bus> <Device> [<numBytes>])\n"); exit(1); }
        
        printf("Bus = %s Device = %s\n", ibus, idev);

        for (bus=busses; bus && (found == 0); bus=bus->next) { // busses loop
                if (strcmp(bus->dirname, ibus) == 0) {
                        for (dev=bus->devices; dev; dev=dev->next) { // devices loop
                                if (strcmp(dev->filename, idev) == 0) { dbus=bus; found=1; break; }
                        }
                }
        }
        
        if (found == 0) { printf("Unable to find the required device !\nexiting\n"); exit(1); }
        
        printf("Found device\n");
        printf("Now we are dealing with device from vendor ID : %d (%x) \n",dev->descriptor.idVendor,dev->descriptor.idVendor);
        printf("Trying to open the device...\n");
        
        if (udev=usb_open(dev)) printf("Device opened successfully.\n");
        else { printf("Operation failed :-("); exit(1);}

        buf=(char*)calloc(1,100);
        if (usb_get_driver_np(udev,0,buf,100)) printf("Could not read the driver name :-( %s\n",buf); 
        else printf("Kernel Using Driver :\n");

        // detach the driver from the kernel , seems to be just like rmmod? 
        if (usb_detach_kernel_driver_np(udev,0)) printf("Error detaching the device :-(\n"); 
        else printf("Device detached successfully from the kernel.\n");

        // reserving the device interface for our application, if another driver/software
        //is using the device , it will return 'interface busy'
        if (r=usb_claim_interface(udev,0)) printf("Interface Claimed !!\n"); 
        printf("Interface Claim Status : %d\n",r);

        printf("Device Protocol : %d\n",dev->descriptor.bDeviceProtocol);
        printf("Report Length : %d\n",dev->descriptor.bLength);
        printf("Decriptor Type : %d\n",dev->descriptor.bDescriptorType);
        printf("End Points : %d\n",dev->config->interface->altsetting->bNumEndpoints);
        printf("Interface Class : %d\n",dev->config->interface->altsetting->bInterfaceClass);
        printf("Protocol : %d\n",dev->config->interface->altsetting->bInterfaceProtocol);
        printf("Interface Number: %d\n",dev->config->interface->altsetting->bInterfaceNumber);
        printf("Device Filename : %s\n",dev->filename);
        printf("Bus Dir Name : %s\n",dbus->dirname);

        usb_get_string_simple(udev,dev->descriptor.iManufacturer,string,sizeof(string));
        printf("Device Manfucaturer : %s\n",string);
        usb_get_string_simple(udev,dev->descriptor.iProduct,string,sizeof(string));
        printf("Product Name : %s\n",string);
        usb_get_string_simple(udev,dev->descriptor.iSerialNumber,string,sizeof(string));
        printf("Device Serial Number: %s\n",string);
        printf("End point addresses : 0x%x\n",dev->config->interface->altsetting->endpoint->bEndpointAddress);

        int numBytes = 4;
        int errCount = 0;
        unsigned char endPoint = 0x81;
	int i;
	int tmp1 = 5;
	int tmp2 = 5;

        // reset usb device if command line numBytes = 0 and exit
        if (argc == 4 && atoi(argv[3]) == 0) { usb_reset(udev); exit(0); }

        // use command line value for numBytes if supplied
        if (argc == 4) numBytes = atoi(argv[3]);
        if (numBytes > 64) numBytes = 64;

        while (1) {  
                for (x=0; x<numBytes; x++)  string[x]=0;
                
                // read numBytes bytes using interrupt_read,
                r = usb_interrupt_read(udev, endPoint, string, numBytes, 0); 
                
                system("clear");
		
		tmp1+=string[1];
		tmp2+=string[2];
		if (tmp1>=0 && tmp2>=0 && tmp1<=150 && tmp2<=50){
		for (i=0;i<tmp2;i++) printf("\n");
		for (i=0;i<tmp1;i++) printf(".");
		if (string[0]==0) printf ("X");
		else if (string[0]==1) printf("L");
		else if(string[0]==2) printf("R");
		else if(string[0]==3) printf("LR");
		else if(string[0]==4) printf("M");
		}
		else {
		  if(tmp1<0) tmp1=0;
		  else if (tmp1>150) tmp1=150;
		  if (tmp2<0) tmp2=0;
		  else if (tmp2>50) tmp2=50;
		}
		if (string[3]==1) printf ("\nSCROLL UP");
		if (string[3]==-1) printf ("\nSCROLL DOWN");
		printf ("\n(%d, %d)",tmp1,tmp2);
		if ( r < 0 ) errCount++;
                if (errCount >= 100) break; 
        
                usb_clear_halt(udev,endPoint); 
        }

        printf("Closing Device.\n");
        usb_release_interface(udev,0);
        usb_close(udev);

        return EXIT_SUCCESS;
}
