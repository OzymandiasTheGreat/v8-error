package to.holepunch;

import com.facebook.react.ReactPackage;
import com.facebook.react.bridge.CatalystInstanceImpl;
import com.facebook.react.bridge.JSIModulePackage;
import com.facebook.react.bridge.JSIModuleSpec;
import com.facebook.react.bridge.JavaScriptContextHolder;
import com.facebook.react.bridge.NativeModule;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.turbomodule.core.CallInvokerHolderImpl;
import com.facebook.react.uimanager.ViewManager;

import androidx.annotation.NonNull;

import java.util.Collections;
import java.util.List;

public class HolepunchBindingsPackage implements JSIModulePackage, ReactPackage {
    @NonNull
    @Override
    public List<NativeModule> createNativeModules(@NonNull ReactApplicationContext reactContext) {
        return Collections.singletonList(new HolepunchBindingsModule(reactContext));
    }

    @NonNull
    @Override
    public List<ViewManager> createViewManagers(@NonNull ReactApplicationContext reactContext) {
        return Collections.singletonList(new KeyboardInsetsAwareViewManager());
    }

    @Override
    public List<JSIModuleSpec> getJSIModules(ReactApplicationContext context,
                                             JavaScriptContextHolder jsContext) {
        CatalystInstanceImpl catalyst = (CatalystInstanceImpl) context.getCatalystInstance();
        CallInvokerHolderImpl callInvoker = (CallInvokerHolderImpl) catalyst.getJSCallInvokerHolder();
        nativeInstall(jsContext.get(), callInvoker, catalyst);
        return Collections.emptyList();
    }

    static {
        try {
            System.loadLibrary("hlp_bindings");
        } catch (Exception ignored) {
        }
    }

    // java > cpp

    private static native void nativeInstall(
            long jsi,
            CallInvokerHolderImpl callInvoker,
            CatalystInstanceImpl catalyst);

    public static native void nativeReloadSignal(long jsi);
}
