package to.holepunch;

import android.app.Activity;

import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.bridge.UiThreadUtil;
import com.facebook.react.module.annotations.ReactModule;

import androidx.annotation.NonNull;

import java.util.HashMap;
import java.util.Map;

@ReactModule(name = HolepunchBindingsModule.NAME)
public class HolepunchBindingsModule extends ReactContextBaseJavaModule {
    public static final String NAME = "HolepunchBindings";

    public HolepunchBindingsModule(ReactApplicationContext reactContext) {
        super(reactContext);
    }

    @NonNull
    @Override
    public String getName() {
        return NAME;
    }

    // https://github.com/itinance/react-native-fs/blob/master/android/src/main/java/com/rnfs/RNFSManager.java

    private static final String FS_BASE = "FS_BASE";
    private static final String FS_TMP_BASE = "FS_TMP_BASE";

    @Override
    public Map<String, Object> getConstants() {
        final Map<String, Object> constants = new HashMap<>();
        constants.put(FS_BASE, this.getReactApplicationContext().getFilesDir().getAbsolutePath());
        constants.put(FS_TMP_BASE, this.getReactApplicationContext().getCacheDir().getAbsolutePath());

        return constants;
    }

    @ReactMethod
    public void hideLaunchScreen() {
        UiThreadUtil.runOnUiThread(() -> {
            Activity activity = getCurrentActivity();
            if (activity == null) {
                return;
            }
            ((WithLaunchScreen)activity).hideLaunchScreen();
        });
    }
}
