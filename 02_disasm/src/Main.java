class Main {
    static int fib(int x) {
        if (x == 0) {
            return 0;
        }
        if (x == 1) {
            return 1;
        }
        int a = 0;
        int b = 1;
        int c = 1;
        for (int n = 2; n < x; ++n) {
            a = b;
            b = c;
            c = a + b;
        }
        return c;
    }

    public static void main(String[] args) {
        for (int i = 0; i < 11; ++i) {
            System.out.println(fib(i));
        }
        System.out.println("Done!");
    }
}
