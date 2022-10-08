package com.explibuv;

import android.os.Bundle;
import android.widget.FrameLayout;

import androidx.core.graphics.Insets;
import androidx.core.splashscreen.SplashScreen;
import androidx.core.splashscreen.SplashScreenViewProvider;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;

import com.facebook.react.ReactActivity;
import com.facebook.react.ReactActivityDelegate;
import com.facebook.react.ReactRootView;

import to.holepunch.WithLaunchScreen;

public class MainActivity extends ReactActivity implements WithLaunchScreen  {
  private static boolean keepLaunchScreen = true;

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    WindowCompat.setDecorFitsSystemWindows(getWindow(), false);
  }

  @Override
  protected String getMainComponentName() {
    return "ExpLibuv";
  }

  public void hideLaunchScreen() {
    keepLaunchScreen = false;
  }

  @Override
  protected ReactActivityDelegate createReactActivityDelegate() {
    return new MainActivityDelegate(this, getMainComponentName());
  }

  public static class MainActivityDelegate extends ReactActivityDelegate {
    public MainActivityDelegate(ReactActivity activity, String mainComponentName) {
      super(activity, mainComponentName);
    }

    @Override
    protected void loadApp(String appKey) {
      SplashScreen splashScreen = SplashScreen.installSplashScreen(getPlainActivity());
      splashScreen.setKeepOnScreenCondition(() -> MainActivity.keepLaunchScreen);
      splashScreen.setOnExitAnimationListener(SplashScreenViewProvider::remove);
      super.loadApp(appKey);
    }

    @Override
    protected ReactRootView createRootView() {
      final ReactRootView reactRootView = new ReactRootView(getContext());
      reactRootView.setIsFabric(BuildConfig.IS_NEW_ARCHITECTURE_ENABLED);

      ViewCompat.setOnApplyWindowInsetsListener(reactRootView, (v, windowInsets) -> {
        Insets insets = windowInsets.getInsets(WindowInsetsCompat.Type.systemBars());

        FrameLayout.LayoutParams lp = (FrameLayout.LayoutParams) v.getLayoutParams();
        lp.topMargin = insets.top;
        lp.leftMargin = insets.left;
        lp.bottomMargin = insets.bottom;
        lp.rightMargin = insets.right;
        v.setLayoutParams(lp);

        return windowInsets;//WindowInsetsCompat.CONSUMED;
      });
      return reactRootView;
    }

    @Override
    protected boolean isConcurrentRootEnabled() {
      // If you opted-in for the New Architecture, we enable Concurrent Root (i.e. React 18).
      // More on this on https://reactjs.org/blog/2022/03/29/react-v18.html
      return BuildConfig.IS_NEW_ARCHITECTURE_ENABLED;
    }
  }
}
