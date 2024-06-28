package com.metallic.chiaki.touchcontrols;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.FrameLayout;

import androidx.annotation.Nullable;
import androidx.appcompat.view.menu.ActionMenuItem;
import androidx.constraintlayout.widget.ConstraintLayout;

/**
 * Description
 * Date: 2024-04-22
 * Time: 14:45
 */
public class CustomView extends View {
    public CustomView(Context context) {
        super(context);
    }

    public CustomView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public CustomView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }


    int lastX = 0;
    int lastY = 0;

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        //获取到手指处的横坐标和纵坐标
//        int x = (int) event.getX();
//        int y = (int) event.getY();
//
//        switch(event.getAction()){
//            case MotionEvent.ACTION_DOWN:
//                lastX = x;
//                lastY = y;
//                break;
//            case MotionEvent.ACTION_MOVE:
//                //计算移动的距离
//                int offX = x - lastX;
//                int offY = y - lastY;
//                //调用layout方法来重新放置它的位置
//                layout(getLeft()+offX, getTop()+offY,
//                        getRight()+offX    , getBottom()+offY);
//                break;
//        }
//        return true;
        return super.onTouchEvent(event);
    }


    public void saveConfiguration(){

    }
}
