import java.util.LinkedList;
import java.util.Queue;


class ProducerRunnable implements Runnable {
    Queue<Integer> buffer;
    ProducerRunnable(Queue<Integer> buffer) {
        this.buffer = buffer;
    }

    public void run() {
        for(int i=0; i<1000; i++) {
            produce(i);
        }
    }

    public void produce(int i) {
        synchronized (buffer) {
            while (buffer.size() == 10) {
                try {
                    System.out.println("PRODUCER: Oh no, buffer is full");
                    buffer.wait();
                } catch (InterruptedException e) {
                    System.out.println("PRODUCER: Oops, producer got interrupted while waiting");
                }
            }
            System.out.println("PRODUCER: Adding item : "+i+" to the buffer");
            buffer.offer(i);
            buffer.notify();
        }
    }
}


class ConsumerRunnable implements Runnable {
    Queue<Integer> buffer;
    ConsumerRunnable(Queue<Integer> buffer) {
        this.buffer = buffer;
    }

    public void run() {
        for(int i=0; i<1000; i++) {
            consume();
        }
    }

    public void consume() {
        synchronized (buffer) {
            while (buffer.isEmpty()) {
                try {
                    System.out.println("CONSUMER: Oh no, buffer is empty()");
                    buffer.wait();
                } catch (InterruptedException e) {
                    System.out.println("CONSUMER: Oops, consumer got interrupted while waiting");
                }
            }
            System.out.println("CONSUMER: removing item : "+ buffer.poll() +" from the buffer");
            buffer.notify();
        }
    }
}

public class ProdConSingle {
    public static void main(String[] args) {
        Queue<Integer> buffer = new LinkedList<>();
        Thread producer = new Thread(new ProducerRunnable(buffer));
        Thread consumer = new Thread(new ConsumerRunnable(buffer));
        producer.start();
        consumer.start();
    }
}
