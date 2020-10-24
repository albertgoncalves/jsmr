class Main {
    static class Struct { public int n; }

    static int fib(Struct x) {
        if (x.n == 0) {
            return 0;
        }
        if (x.n == 1) {
            return 1;
        }
        int a = 0;
        int b = 1;
        int c = 1;
        for (int m = 2; m < x.n; ++m) {
            a = b;
            b = c;
            c = a + b;
        }
        return c;
    }

    public static void main(String[] args) {
        for (int i = 0; i < 11; ++i) {
            Struct struct = new Struct();
            struct.n = i;
            System.out.println(fib(struct));
        }
        System.out.println("Done!");
    }
}
