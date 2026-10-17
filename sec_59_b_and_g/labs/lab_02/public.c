int main() {
    int count = 3;
    int total = 0;
    // Add the remaining count.
    while (count > 0) {
        total = total + count;
        count = count - 1;
    }
    if (total >= 6 && count == 0) {
        return total;
    } else {
        return 0;
    }
}
