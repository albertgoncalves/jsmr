class Main {
    static int ackermann(int m, int n) {
        if (m == 0) {
            return n + 1;
        } else if (n == 0) {
            return ackermann(m - 1, 1);
        } else {
            return ackermann(m - 1, ackermann(m, n - 1));
        }
    }

    public static void main(String[] args) {
        for (int i = 0; i < 12; ++i) {
            System.out.println(ackermann(3, i));
        }
        System.out.println("Done!");
    }
}
