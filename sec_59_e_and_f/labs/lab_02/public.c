int main() {
    int steps = 4;
    int result = 1;
    // Multiply by the remaining steps.
    while (steps > 1) {
        result = result * steps;
        steps = steps - 1;
    }
    if (result >= 24 && steps == 1) {
        return result;
    } else {
        return 0;
    }
}
