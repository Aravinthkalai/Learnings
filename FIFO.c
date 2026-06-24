#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/********************************************************
 * FIFO Definition
 ********************************************************/

K_FIFO_DEFINE(sensor_fifo);

/********************************************************
 * FIFO Data Structure
 *
 * IMPORTANT:
 * First member MUST be reserved pointer
 ********************************************************/

struct sensor_data
{
    void *fifo_reserved;

    uint32_t temperature;
    uint32_t pressure;
};

/********************************************************
 * Producer Thread
 ********************************************************/

void producer_thread(void)
{
    static struct sensor_data sensor;

    while (1)
    {
        sensor.temperature += 1;
        sensor.pressure += 2;

        printk("[PRODUCER] Temp=%d Pressure=%d\n",
               sensor.temperature,
               sensor.pressure);

        /*
         * Put pointer into FIFO
         */

        k_fifo_put(&sensor_fifo,
                   &sensor);

        k_sleep(K_SECONDS(1));
    }
}

/********************************************************
 * Consumer Thread
 ********************************************************/

void consumer_thread(void)
{
    struct sensor_data *rx_data;

    while (1)
    {
        /*
         * Wait for FIFO data
         */

        rx_data = k_fifo_get(&sensor_fifo,
                             K_FOREVER);

        printk("[CONSUMER] Temp=%d Pressure=%d\n",
               rx_data->temperature,
               rx_data->pressure);
    }
}

/********************************************************
 * Threads
 ********************************************************/

K_THREAD_DEFINE(prod_tid,
                1024,
                producer_thread,
                NULL,NULL,NULL,
                5,
                0,
                0);

K_THREAD_DEFINE(cons_tid,
                1024,
                consumer_thread,
                NULL,NULL,NULL,
                5,
                0,
                0);

int main(void)
{
    printk("FIFO Example\n");

    while(1)
    {
        k_sleep(K_SECONDS(10));
    }
}