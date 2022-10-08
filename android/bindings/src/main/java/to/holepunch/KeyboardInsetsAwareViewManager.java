package to.holepunch;

import androidx.annotation.NonNull;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.facebook.react.uimanager.DisplayMetricsHolder;
import com.facebook.react.uimanager.LayoutShadowNode;
import com.facebook.react.uimanager.Spacing;
import com.facebook.react.uimanager.ThemedReactContext;
import com.facebook.react.uimanager.UIManagerModule;
import com.facebook.react.uimanager.ViewGroupManager;
import com.facebook.react.views.view.ReactViewGroup;

public class KeyboardInsetsAwareViewManager extends ViewGroupManager<ReactViewGroup> {
    public static final String REACT_CLASS = "KeyboardInsetsAwareView";

    private static final class LocalData {
        float paddingBottom;
    }

    public static class ViewShadowView extends LayoutShadowNode {
        @Override
        public void setLocalData(Object data) {
            LocalData localData = (LocalData)data;
            setPadding(Spacing.BOTTOM, localData.paddingBottom);
        }
    }

    @NonNull
    @Override
    public String getName() {
        return REACT_CLASS;
    }

    @NonNull
    @Override
    protected ReactViewGroup createViewInstance(final @NonNull ThemedReactContext context) {
        ReactViewGroup view = new ReactViewGroup(context);
        ViewCompat.setOnApplyWindowInsetsListener(view, (v, windowInsets) -> {
            UIManagerModule uiManager = context.getNativeModule(UIManagerModule.class);
            int id = view.getId();
            if (uiManager != null && id >= 0) {
                LocalData localData = new LocalData();
                boolean imeVisible = windowInsets.isVisible(WindowInsetsCompat.Type.ime());
                if (imeVisible) {
                    int screenHeight = DisplayMetricsHolder.getScreenDisplayMetrics().heightPixels;

                    int imeHeight = windowInsets.getInsets(WindowInsetsCompat.Type.ime()).bottom;

                    int[] location = new int[2];
                    view.getLocationOnScreen(location);
                    int viewSpace = location[1] + view.getHeight();

                    localData.paddingBottom = imeHeight - (screenHeight - viewSpace);
                }
                else {
                    localData.paddingBottom = 0;
                }
                uiManager.setViewLocalData(id, localData);
            }

            return WindowInsetsCompat.CONSUMED;
        });
        return view;
    }

    @Override
    public ViewShadowView createShadowNodeInstance() {
        return new ViewShadowView();
    }
}
