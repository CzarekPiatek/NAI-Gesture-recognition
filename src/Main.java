import org.opencv.core.Core;

public class Main {
    public static void main(String[] args) {
        // Load the native OpenCV library
        System.loadLibrary(Core.NATIVE_LIBRARY_NAME);

        new ObjectRecognition().run(args);
    }
}