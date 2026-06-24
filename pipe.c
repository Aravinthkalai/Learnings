#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/********************************************************
 * Pipe Definition
 ********************************************************/

K_PIPE_DEFINE(my_pipe,
              128,
              4);

/********************************************************
 * Producer
 ********************************************************/

void pipe_writer_thread(void)
{
    char tx_data[] = "HELLO_PIPE";

    size_t bytes_written;

    while (1)
    {
        /*
         * Write bytes into pipe
         */

        k_pipe_put(&my_pipe,
                   tx_data,
                   sizeof(tx_data),
                   &bytes_written,
                   sizeof(tx_data),
                   K_FOREVER);

        printk("[WRITER] Sent %d bytes\n",
               bytes_written);

        k_sleep(K_SECONDS(3));
    }
}

/********************************************************
 * Consumer
 ********************************************************/

void pipe_reader_thread(void)
{
    char rx_buffer[20];

    size_t bytes_read;

    while (1)
    {
        /*
         * Read bytes from pipe
         */

        k_pipe_get(&my_pipe,
                   rx_buffer,
                   sizeof(rx_buffer),
                   &bytes_read,
                   sizeof("HELLO_PIPE"),
                   K_FOREVER);

        printk("[READER] %s\n",
               rx_buffer);
    }
}

/********************************************************
 * Threads
 ********************************************************/

K_THREAD_DEFINE(writer_tid,
                1024,
                pipe_writer_thread,
                NULL,NULL,NULL,
                5,
                0,
                0);

K_THREAD_DEFINE(reader_tid,
                1024,
                pipe_reader_thread,
                NULL,NULL,NULL,
                5,
                0,
                0);

int main(void)
{
    printk("Pipe Example\n");

    while(1)
    {
        k_sleep(K_SECONDS(10));
    }
}