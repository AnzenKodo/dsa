import java.util.*;

public class QuickFindUF {
    private int[] id;
    private int count;

    QuickFindUF(int count) {
        this.count = count;
        this.id = new int[count];
        for (int i = 0; i < count; i++) {
            this.id[i] = i;
        }
    }
    public int count() {
        return this.count;
    }

    public void validate(int p) {
        int n = this.id.length;
        if (p < 0 || p >= n) {
            throw new IllegalArgumentException("index " + p + " is not between 0 and " + (n-1));
        }
    }

    public int find(int p) {
        validate(p);
        return this.id[p];
    }

    public void union(int p, int q) {
        validate(p);
        validate(q);
        int pId = this.id[p];
        int qId = this.id[q];

        if (pId != qId) {
            for (int i = 0; i < this.id.length; i++) {
                if (id[i] == pId) {
                    id[i] = qId;
                }
            }
            count--;
        }
    }

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        int n = scanner.nextInt();
        QuickFindUF uf = new QuickFindUF(n);
        while (scanner.hasNext()) {
            int p = scanner.nextInt();
            int q = scanner.nextInt();
            if (uf.find(p) != uf.find(q)) {
                uf.union(p, q);
                System.out.println(p + " "  + q);
            }
        }
        System.out.println(uf.count() + " components");
    }
}
