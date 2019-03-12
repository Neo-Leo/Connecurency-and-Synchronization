import java.util.LinkedList;
import java.util.Queue;
import java.util.Random;


class CustomBlockingQueue<E> {

    Queue<E> queue;
    Object notFull, notEmpty, lock;
    int capacity;

    CustomBlockingQueue(int capacity) {
        this.capacity = capacity;
        queue = new LinkedList();
    }

    public void put(E i) {
        synchronized(queue) {
            while (queue.size() == capacity) {
                try {
                    System.out.println("PRODUCER: Oh no, buffer is full");
                    queue.wait();
                } catch (InterruptedException e) {
                    System.out.println("PRODUCER: Oops, producer got interrupted while waiting");
                }
            }
            System.out.println("PRODUCER: Adding item : " + i + " to the buffer");
            queue.offer(i);
            queue.notifyAll();
        }
    }

    public synchronized E take() {
        synchronized (queue) {
            while (queue.isEmpty()) {
                try {
                    System.out.println("CONSUMER: Oh no, buffer is empty()");
                    queue.wait();
                } catch (InterruptedException e) {
                    System.out.println("CONSUMER: Oops, consumer got interrupted while waiting");
                }
            }
            System.out.println("CONSUMER: removing item : " + queue.poll() + " from the buffer");
            E val = queue.poll();
            queue.notifyAll();
            return val;
        }
    }
}

public class ProdConMulti {
    public static void main(String[] args) {
        CustomBlockingQueue<Integer> queue = new CustomBlockingQueue<>(10);
        Runnable producer = () -> {
            Random r = new Random();
            while(true) {
                queue.put(r.nextInt(100));
            }
        };

        Runnable consumer = () -> {
            while (true) {
                System.out.println("Item "+ queue.take() + " taken");
            }
        };

        Thread p1 = new Thread(producer);
        Thread p2 = new Thread(producer);
        Thread c1 = new Thread(consumer);
        Thread c2 = new Thread(consumer);

        p1.start();
        p2.start();
        c1.start();
        c2.start();
    }
}
