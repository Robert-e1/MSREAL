#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#define BUFF_SIZE 100
MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

DECLARE_WAIT_QUEUE_HEAD(readQ);
DECLARE_WAIT_QUEUE_HEAD(writeQ);
struct semaphore sem;


char stred[100];
char upis[] = "string=";
char brisanje[] = "clear";
char brisanje_space[] = "shrink";
char dodavanje[] = "append=";
char brisanje_br_kar[] = "truncate=";
char brisanje_pojave[] = "remove=";

int pos = 0;
int endRead = 0;

int stred_open(struct inode *pinode, struct file *pfile);
int stred_close(struct inode *pinode, struct file *pfile);
ssize_t stred_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t stred_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);


struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = stred_open,
	.read = stred_read,
	.write = stred_write,
	.release = stred_close,
};

char *strremove(char *str, const char *sub)   //f-ja za brisanje pojave niza karaktera
{
  int len = strlen(sub);
  if (len>0)
    {
      char *p = str;
      while ((p = strstr(p,sub)) != NULL)
	{
	  memmove(p, p+ len, strlen(p+len) + 1);
	}
    }
  return str;
}

int stred_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened stred\n");
		return 0;
}

int stred_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed stred\n");
		return 0;
}

ssize_t stred_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int ret;
	char buff[BUFF_SIZE];
	long int len = 0;
	if (endRead){
		endRead = 0;
		return 0;
	}

	if(down_interruptible(&sem))
		return -ERESTARTSYS;
	while(pos <= 0)
	{
		up(&sem);
		if(wait_event_interruptible(readQ,(pos>0)))
			return -ERESTARTSYS;
		if(down_interruptible(&sem))
			return -ERESTARTSYS;
	}


	if(pos > 0)
	{
		len = scnprintf(buff, BUFF_SIZE, "%s \n", stred);
		ret = copy_to_user(buffer, buff, len);       //procesi iz korisnickog prostora\
nemaju pristup memorijskim adresama kernela, te se niz iz kernela kopira u korisnicki adresni\
prostor (copy_to_user)(return: broj NEuspesno kopiranih bajtova)
		
		if(ret)
			return -EFAULT;
		printk(KERN_INFO "Succesfully read\n");
		endRead = 1;
	}
	else
	{
			printk(KERN_WARNING "Failed to read \n"); 
	}

	up(&sem);
	wake_up_interruptible(&writeQ);

	return len;
}

ssize_t stred_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
	char buff[BUFF_SIZE];
	char input[BUFF_SIZE];
	int ret;

	ret = copy_from_user(buff, buffer, length); //slicno kao kod copy_to_user(return: \
broj NEuspesno kopiranih bajtova)
	if(ret)
		return -EFAULT;
	buff[length-1] = '\0';

	//if(down_interruptible(&sem))
	//	return -ERESTARTSYS;
	//while(pos >= 100)
	//{
	//	up(&sem);
	//	if(wait_event_interruptible(writeQ,(pos<100)))
	//		return -ERESTARTSYS;
	//	if(down_interruptible(&sem))
	//		return -ERESTARTSYS;
	//}

	if(pos<100)
	{
	  int i = 0;
	  for(i=0; i<length; i++)
	    input[i] = buff[i];
	    // ret = sscanf(buff,"%s",input); <- ne radi?
	  ret = length;
	  input[ret-1] = '\0';

	      if(!strncmp(input,upis,strlen(upis))){   //UPIS STRINGA U BAFER
		if(ret-strlen(upis)<= 100-pos)   //ima dovoljno mesta u nizu za string
		{
		  printk(KERN_INFO "Succesfully wrote string "); 
		        stred[0] = '\0';
		        for(i=0; i<ret-strlen(upis); i++)
			  {
			    stred[i] = input[i+strlen(upis)];
			  }
			pos=strlen(stred);
			stred[pos] = '\0';
			printk(KERN_INFO "pos = %d",pos);
		}
		else
		{
			printk(KERN_WARNING "Failed to write string: string is too long!\n");
		}
	      }else if(!strncmp(input,brisanje,strlen(brisanje))) //BRISANJE CITAVOG STRINGA
	      {
		stred[0] = '\0';
		pos = 0;
		printk(KERN_INFO "pos = %d",pos);
	      }else if(!strncmp(input,brisanje_space,strlen(brisanje_space)))//BRISANJE SPACE-ova
	      {
		int i = 0;
		while(stred[0] == ' ')
		  {
		    for(i=0;i<pos;i++)
		      {
			stred[i] = stred[i+1];
		      }
		    pos--;
		    stred[pos] = '0';
		  }
		while(stred[pos-1]== ' ')
		{
		  stred[pos-1] = '\0';
		  pos--;
		}
		stred[pos] = '\0';
		printk(KERN_INFO "String succesfully shrunk!\n");
		printk(KERN_INFO "pos= %d \n",pos);
		
	      }else if(!strncmp(input,dodavanje,strlen(dodavanje)))//KONKATENACIJA
	      {
	   
		  if(down_interruptible(&sem))
			  return -ERESTARTSYS;
		  while(pos >= 100 || length-strlen(dodavanje) >= 100-pos)
	                {
		          up(&sem);
		  if(wait_event_interruptible(writeQ,(pos<100 || ret-strlen(dodavanje) < 100-pos)))
			      return -ERESTARTSYS;
		  if(down_interruptible(&sem))
			      return -ERESTARTSYS;
	                }

		  printk(KERN_INFO "String appended succesfully!\n");
     
		  for(i=0;i<ret-strlen(dodavanje);i++)
		    {
		      //printk(KERN_INFO "%c",input[i+strlen(dodavanje)]);
		      stred[pos+i] = input[i+strlen(dodavanje)];
		    }
		        pos = pos + ret-strlen(dodavanje)-1;
			printk(KERN_INFO "pos= %d",pos);
			//}
			//else
			//{
			//printk(KERN_WARNING "Failed to write string: string will be appended once enough space is available\n");
			//	        }

	      }else if(!strncmp(input,brisanje_br_kar,strlen(brisanje_br_kar)))//BRISANJE KARAKTERA
	      {
		int br=0;
		char dummy[] = {0};
		sscanf(input,"%s" "%d",dummy, &br);
		pos = pos-br;
		if(pos < 0)
		  pos = 0;
		stred[pos] = '\0';
		printk(KERN_INFO "Succesfully deleted last %d characters!\n",br);
		printk(KERN_INFO "pos = %d",pos);
	      }else if(!strncmp(input,brisanje_pojave,strlen(brisanje_pojave)))//BRISANJE POJAVE
	      {
		char needle[] = {0};
		for(i=0;i<strlen(input)-strlen(brisanje_pojave);i++)
		  {
		    needle[i] = input[i+strlen(brisanje_pojave)];
		  }
		needle[strlen(input)-strlen(brisanje_pojave)] = '\0';
		strremove(stred,needle);
		pos = strlen(stred);
		printk(KERN_INFO "Succesfully removed substring %s",needle);
		printk(KERN_INFO "pos = %d",pos);
	      }else
              {
		printk(KERN_WARNING "Lose formatirana komanda!\n");
	      }//kraj bloka za formtiranje stringa
	}
	      
	else
	{
		printk(KERN_WARNING "Buffer is full\n"); 
	}

	up(&sem);
	wake_up_interruptible(&readQ);

	return length;
}

static int __init stred_init(void)
{
   int ret = 0;
   //	int i=0;
	
	sema_init(&sem,1);

	//Initialize array
	stred[0] = '\0';
	//	for (i=0; i<100; i++)
	//	stred[i] = 0;

   ret = alloc_chrdev_region(&my_dev_id, 0, 1, "stred");
   if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "stred_class");
   if (my_class == NULL){
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, "stred");
   if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
   printk(KERN_INFO "device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret)
	{
      printk(KERN_ERR "failed to add cdev\n");
		goto fail_2;
	}
   printk(KERN_INFO "cdev added\n");
   printk(KERN_INFO "Hello world\n");

   return 0;

   fail_2:
      device_destroy(my_class, my_dev_id);
   fail_1:
      class_destroy(my_class);
   fail_0:
      unregister_chrdev_region(my_dev_id, 1);
   return -1;
}

static void __exit stred_exit(void)
{
   cdev_del(my_cdev);
   device_destroy(my_class, my_dev_id);
   class_destroy(my_class);
   unregister_chrdev_region(my_dev_id,1);
   printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(stred_init);
module_exit(stred_exit);
