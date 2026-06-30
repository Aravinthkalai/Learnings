/*
 * Zephyr RTOS Complete Interview Revision Example
 *
 * Covers:
 * ==========
 * 1. K_THREAD_DEFINE()      (Static Thread Creation)
 * 2. k_thread_create()      (Dynamic Thread Creation)
 * 3. k_msgq                (Message Queue)
 * 4. k_mutex              (Shared Resource Protection)
 * 5. k_sem                (Thread Synchronization)
 * 6. k_mbox               (Mailbox Communication)
 * 7. k_work               (Work Queue)
 * 8. ISR -> Thread Notification
 * 9. Thread Priorities
 * 10. Shared Resource Access From Multiple Contexts
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/****************************************************************
 * MESSAGE QUEUE
 ****************************************************************/

/*
 * K_MSGQ_DEFINE(
 *      name,
 *      message_size,
 *      max_messages,
 *      alignment
 * );
 */
/*Most MCUs access memory faster when data is aligned to its natural boundary.
so keep sizeof(data_type) and it is multiples like if uint32_t then alignment should be 4, 
uint8_t then alignment should be 1*/

K_MSGQ_DEFINE(sensor_msgq,
              sizeof(int),
              10,
              4);

/****************************************************************
 * MUTEX
 ****************************************************************/

/*
 * Protects shared resource display_buffer
 */

K_MUTEX_DEFINE(display_mutex);

/****************************************************************
 * SEMAPHORE
 ****************************************************************/

/*
 * Used to synchronize Producer and Consumer
 */

K_SEM_DEFINE(sensor_sem,
             0,
             1);

/****************************************************************
 * MAILBOX
 ****************************************************************/

K_MBOX_DEFINE(status_mbox);

/****************************************************************
 * WORK QUEUE
 ****************************************************************/

/*
 * Work item object
 */

static struct k_work sensor_work;

/****************************************************************
 * SHARED RESOURCE
 ****************************************************************/

/*
 * Shared by:
 *
 * Work Queue Handler
 * Dynamic Thread
 * Main Thread
 *
 * Must be protected using mutex
 */

int display_buffer = 0;

/****************************************************************
 * DYNAMIC THREAD RESOURCES
 ****************************************************************/

/*
 * Stack allocation for dynamic thread
 */

K_THREAD_STACK_DEFINE(dynamic_stack,
                      1024);

/*
 * Thread Control Block (TCB)
 */

struct k_thread dynamic_thread_data;

/****************************************************************
 * WORK QUEUE HANDLER
 ****************************************************************/

/*
 * Executes in Work Queue Thread Context
 *
 * Triggered using:
 * k_work_submit()
 */

void sensor_work_handler(struct k_work *work)
{
    printk("\n[WORKQUEUE] Running\n");

    /*
     * Lock mutex before modifying
     * shared resource
     */

    k_mutex_lock(&display_mutex,
                 K_FOREVER);

    display_buffer++;

    printk("[WORKQUEUE] Updated display_buffer=%d\n",
           display_buffer);

    /*
     * Unlock mutex
     */

    k_mutex_unlock(&display_mutex);
}

/****************************************************************
 * PRODUCER THREAD
 ****************************************************************/

void producer_thread(void)
{
    int sensor_value = 0;

    while (1)
    {
        sensor_value++;

        /*
         * Add data to queue
         */

        k_msgq_put(&sensor_msgq,
                   &sensor_value,
                   K_NO_WAIT);

        printk("[PRODUCER] Produced=%d\n",
               sensor_value);

        /*
         * Notify consumer
         */

        k_sem_give(&sensor_sem);

        k_sleep(K_SECONDS(1));
    }
}

/****************************************************************
 * CONSUMER THREAD
 ****************************************************************/

void consumer_thread(void)
{
    int value;

    while (1)
    {
        /*
         * Wait until producer gives semaphore
         */

        k_sem_take(&sensor_sem,
                   K_FOREVER);

        /*
         * Read queue data
         */

        if (k_msgq_get(&sensor_msgq,
                       &value,
                       K_NO_WAIT) == 0)
        {
            printk("[CONSUMER] Received=%d\n",
                   value);
        }

        /*
         * Submit work item
         *
         * DOES NOT CALL HANDLER DIRECTLY
         * Scheduler later executes handler
         * in Work Queue Thread Context
         */

        k_work_submit(&sensor_work);
    }
}

/****************************************************************
 * MAILBOX TX THREAD
 ****************************************************************/

void mailbox_tx_thread(void)
{
    static char status[] = "SYSTEM_OK";

    struct k_mbox_msg msg;

    while (1)
    {
        msg.info = 1;
        msg.size = sizeof(status);
        msg.tx_data = status;

        k_mbox_put(&status_mbox,
                   &msg,
                   K_FOREVER);

        printk("[MAILBOX TX] Sent\n");

        k_sleep(K_SECONDS(5));
    }
}

/****************************************************************
 * MAILBOX RX THREAD
 ****************************************************************/

void mailbox_rx_thread(void)
{
    struct k_mbox_msg msg;

    char rx_buffer[20];

    while (1)
    {
        k_mbox_get(&status_mbox,
                   &msg,
                   rx_buffer,
                   K_FOREVER);

        printk("[MAILBOX RX] %s\n",
               rx_buffer);
    }
}

/****************************************************************
 * DYNAMIC THREAD FUNCTION
 ****************************************************************/

/*
 * Created at runtime using
 * k_thread_create()
 *
 * Reads display_buffer
 * Therefore mutex is required
 */

void dynamic_thread(void *arg1,
                    void *arg2,
                    void *arg3)
{
    int local_copy;

    while (1)
    {
        /*
         * Lock mutex
         */

        k_mutex_lock(&display_mutex,
                     K_FOREVER);

        local_copy = display_buffer;

        printk("[DYNAMIC THREAD] display_buffer=%d\n",
               local_copy);

        /*
         * Unlock mutex
         */

        k_mutex_unlock(&display_mutex);

        k_sleep(K_SECONDS(3));
    }
}

/****************************************************************
 * SIMULATED ISR
 ****************************************************************/

/*
 * Example only
 *
 * ISR should be short
 */

void fake_gpio_isr(void)
{
    printk("[ISR] Interrupt\n");

    /*
     * Wake consumer thread
     */

    k_sem_give(&sensor_sem);

    /*
     * Submit work
     *
     * Heavy processing deferred
     */

    k_work_submit(&sensor_work);
}

/****************************************************************
 * STATIC THREADS
 ****************************************************************/

/*
 * Created automatically
 * before main()
 */

K_THREAD_DEFINE(producer_tid,
                1024,
                producer_thread,
                NULL,NULL,NULL,
                5,
                0,
                0);

K_THREAD_DEFINE(consumer_tid,
                1024,
                consumer_thread,
                NULL,NULL,NULL,
                4,
                0,
                0);

K_THREAD_DEFINE(mailbox_tx_tid,
                1024,
                mailbox_tx_thread,
                NULL,NULL,NULL,
                6,
                0,
                0);

K_THREAD_DEFINE(mailbox_rx_tid,
                1024,
                mailbox_rx_thread,
                NULL,NULL,NULL,
                6,
                0,
                0);

/****************************************************************
 * MAIN
 ****************************************************************/

int main(void)
{
    k_tid_t dynamic_tid;

    printk("\n===== ZEPHYR RTOS DEMO =====\n");

    /********************************************************
     * Initialize Work Item
     ********************************************************/

    k_work_init(&sensor_work,
                sensor_work_handler);

    /********************************************************
     * Create Dynamic Thread
     ********************************************************/

    dynamic_tid = k_thread_create(
                        &dynamic_thread_data,
                        dynamic_stack,
                        K_THREAD_STACK_SIZEOF(dynamic_stack),
                        dynamic_thread,
                        NULL,
                        NULL,
                        NULL,
                        7,
                        0,
                        K_NO_WAIT);

    printk("Dynamic Thread Created\n");

    while (1)
    {
        int current_value;

        /*
         * Main thread also reads
         * display_buffer
         */

        k_mutex_lock(&display_mutex,
                     K_FOREVER);

        current_value = display_buffer;

        printk("[MAIN] display_buffer=%d\n",
               current_value);

        k_mutex_unlock(&display_mutex);

        /*
         * Examples:
         */

        /*
        k_thread_suspend(dynamic_tid);
        */

        /*
        k_thread_resume(dynamic_tid);
        */

        /*
        k_thread_priority_set(dynamic_tid,
                              3);
        */

        k_sleep(K_SECONDS(10));
    }

    return 0;
}

/*Semaphore flow

Semaphore Count = 0

Consumer
   |
   +--> k_sem_take()
   |
   +--> BLOCKED

--------------------------------

Producer
   |
   +--> k_sem_give()
   |
Count 0 -> 1

--------------------------------

RTOS wakes Consumer

Consumer
   |
Count 1 -> 0
   |
Processing Data

*/


/*Mutex explanation let say Thread A got the lock first

Thread A
   |
   |
k_mutex_lock()
   |
   V
Mutex Locked
   |
   |
Update Resource
   |
   |
k_mutex_unlock()
   |
   V
Mutex Released

-------------------------

Thread B
   |
k_mutex_lock()
   |
Mutex Busy
   |
BLOCKED
   |
Wait...
   |
Mutex Released
   |
READY
   |
RUNNING
*/


/*Work queue notes
It is also a thread called work queue thread, whenever we call k_work_submit()
the work will be queued and it will be executed in sequence order like first k_work_submit() will be executed first and so on. 
The work queue thread has a priority of 0 by default, but we can change it using K_WORK_Q_DEFINE() macro.
we can do all the thread works using work queue as like below
K_WORK_DEFINE(display_work, display_handler);
K_WORK_DEFINE(logger_work, logger_handler);
K_WORK_DEFINE(usb_work, usb_handler);
K_WORK_DEFINE(sensor_work, sensor_handler);

When needed:
k_work_submit(&display_work);
k_work_submit(&logger_work);
k_work_submit(&usb_work);
k_work_submit(&sensor_work);

Internally:
Work Queue
----------------
display_work
logger_work
usb_work
sensor_work

One worker thread processes them one by one.

Worker Thread
      |
      +--> display_handler()
      |
      +--> logger_handler()
      |
      +--> usb_handler()
      |
      +--> sensor_handler()

When Work Queue is Good
Button Press Event
USB Connected Event
Log Message Event
LED Update Event
These happen occasionally.

Using dedicated threads would waste RAM.

Work queue is perfect.


*/

/*
| Feature             | k_msgq | k_fifo | k_pipe |
| ------------------- | ------ | ------ | ------ |
| Copies Data         | Yes    | No     | Yes    |
| Fixed Size Messages | Yes    | No     | No     |
| Variable Size       | No     | Yes    | Yes    |
| Stream Data         | No     | No     | Yes    |
| Uses Pointer        | No     | Yes    | No     |
| Most Common         | ⭐⭐⭐⭐⭐  | ⭐⭐⭐⭐   | ⭐⭐⭐    |


I can, but for large buffers k_msgq performs memory copies. 
k_fifo avoids copies by passing pointers, which reduces CPU overhead and memory bandwidth usage. 
For stream-oriented data such as UART or firmware downloads, 
k_pipe is more suitable because it handles variable-length byte streams naturally.

msg_queue:
Data is COPIED into Zephyr's queue buffer.
let say 4kb of data then put side 4k and get side 4k total 8kb memory used

k_fifo:
Passes put side pointer to get side, so only 4kb memory used

k_pipe:
Data is COPIED into Zephyr's pipe buffer.
4KB in 4KB out, so 8KB memory used
Why pipes are useful for stream-oriented data, such as UART or firmware downloads,
because they handle variable-length byte streams naturally.


*/