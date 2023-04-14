package com.example.inout_nodeapp;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

import static com.example.inout_nodeapp.MmenuIconContentOrientation.CIRCLE;
import static com.example.inout_nodeapp.MmenuIconContentOrientation.LEFT;
import static com.example.inout_nodeapp.MmenuIconContentOrientation.RIGHT;

@Retention(RetentionPolicy.CLASS)
@interface MmenuIconContentOrientation {
    int LEFT = 0;
    int RIGHT = 1;
    int CIRCLE = 2;
    int CIRCLETOP = 3;
}

public class MmenuIconContent extends ConstraintLayout {
    private int orientation = CIRCLE;
    private Drawable iconSrc = getResources().getDrawable(R.mipmap.ic_launcher_foreground);
    private String Title = "Setting";
    private String subTitle = "subTitle";
    private int IconSize = 200;
    private float TextSize = 20.f;
    private float subTitleSize = 10.f;
    private int imageSrc = R.drawable.attend_list_item_door;
    private int MarginVertical = 50;
    private int MarginLeft = 50;
    private int MarginRight = 50;
    private int TextColor = 0x000000;
    private int subTitleColor = 0x000000;
    private boolean subTitleVisible = true;

    final int[] roundimage_array = {R.drawable.menuicon_contents_round_left,R.drawable.menuicon_contents_round_right,R.drawable.menuicon_contents_round_circle,R.drawable.menuicon_contents_round_circle};

    private ImageView Icon;
    private ConstraintLayout Background;
    private TextView Titleview,subTitleview;
    private int BackColor = Color.parseColor("#FF6200EE");

    public MmenuIconContent(Context context) {
        super(context);
        init(null, 0);
    }

    public MmenuIconContent(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(attrs, 0);
    }

    public MmenuIconContent(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init(attrs, defStyle);
    }

    private void init(AttributeSet attrs, int defStyle) {
        // Load attributes
        final TypedArray a = getContext().obtainStyledAttributes(
                attrs, R.styleable.MmenuIconContent, defStyle, 0);

        orientation = a.getInt(R.styleable.MmenuIconContent_ContentOrientation,0);
        if (a.hasValue(R.styleable.MmenuIconContent_iconSrc)) {
            iconSrc = a.getDrawable(R.styleable.MmenuIconContent_iconSrc);
            iconSrc.setCallback(this);
        }
        Title = a.getString(R.styleable.MmenuIconContent_Title);
        subTitle = a.getString(R.styleable.MmenuIconContent_subTitle);

        BackColor = a.getColor(
                R.styleable.MmenuIconContent_BackColor,BackColor);
        IconSize = a.getDimensionPixelSize(
                R.styleable.MmenuIconContent_IconSize,IconSize);
        TextSize = a.getDimension(
                R.styleable.MmenuIconContent_TextSize,TextSize);
        imageSrc = a.getResourceId(
                R.styleable.MmenuIconContent_iconSrc,imageSrc);
        MarginVertical = a.getDimensionPixelOffset(
                R.styleable.MmenuIconContent_MarginVertical,MarginVertical);
        MarginLeft = a.getDimensionPixelOffset(
                R.styleable.MmenuIconContent_MarginLeft,MarginLeft);
        MarginRight = a.getDimensionPixelOffset(
                R.styleable.MmenuIconContent_MarginRight,MarginRight);
        TextColor = a.getColor(
                R.styleable.MmenuIconContent_textColor,TextColor);
        subTitleSize = a.getDimension(
                R.styleable.MmenuIconContent_SubTitleSize,subTitleSize);
        subTitleColor = a.getColor(
                R.styleable.MmenuIconContent_subTitleColor,subTitleColor);
        subTitleVisible = a.getBoolean(
                R.styleable.MmenuIconContent_subTitleVisible,true);


        String inflaterService = Context.LAYOUT_INFLATER_SERVICE;
        LayoutInflater layoutInflater = (LayoutInflater) getContext().getSystemService(inflaterService);


        if(0 > orientation || orientation >= 4)return;
        int[] layoutRes_array = {R.layout.menuicon_content_view,R.layout.menuicon_content_view_right,R.layout.menuicon_content_view_circle,R.layout.menuicon_content_view_circle};

        View view = layoutInflater.inflate(layoutRes_array[orientation], MmenuIconContent.this, false);
        addView(view);

        Icon = findViewById(R.id.Image);
        Drawable backcircle = getResources().getDrawable(R.drawable.menuicon_contents_round_circle);
        backcircle.setColorFilter(Color.WHITE, PorterDuff.Mode.SRC_OVER);

        Icon.setImageResource(imageSrc);

        //ShapeDrawable shape = new ShapeDrawable(new OvalShape());
        //shape.setColorFilter(Color.WHITE, PorterDuff.Mode.SRC_OVER);
        //Icon.setBackground(shape);
        Icon.setBackground(backcircle);
        //Icon.setBackgroundColor(Color.WHITE);

        //Icon.setPadding(MarginLeft,0,MarginRight,0);
        LayoutParams lp = (LayoutParams) Icon.getLayoutParams();
        lp.width = IconSize;
        lp.height = IconSize;
        lp.leftMargin = MarginLeft;
        lp.rightMargin = MarginRight;
        if(orientation == CIRCLE)
        {
            lp.topMargin = MarginVertical;
            lp.bottomMargin = MarginVertical;
        }
        //lp.setMargins(MarginLeft, MarginVertical, MarginRight, MarginVertical);
        Icon.setLayoutParams(lp);

        if(orientation != CIRCLE)
        {
            LinearLayout TextGroup = findViewById(R.id.textGroup);
            LayoutParams layoutParams = (ConstraintLayout.LayoutParams)TextGroup.getLayoutParams();
            if(orientation == LEFT)//<==
                layoutParams.leftMargin = MarginRight;
            else if(orientation == RIGHT)//==>
                layoutParams.rightMargin = MarginLeft;
            TextGroup.setLayoutParams(layoutParams);
        }

        setBackgroundColor(BackColor);
        //Background.setBackgroundColor(BackColor);
        //Background.setClipToOutline(true);
        //lp = (LayoutParams) Background.getLayoutParams();
        //lp.width = ViewGroup.LayoutParams.MATCH_PARENT;
        //lp.height = ViewGroup.LayoutParams.MATCH_PARENT;
        //Background.setLayoutParams(lp);

        //Title = "Title";

        Titleview = findViewById(R.id.Title);
        Titleview.setTextSize(TextSize);
        Titleview.setText(Title);
        Titleview.setTextColor(TextColor);
        Titleview.setTypeface(Typeface.DEFAULT_BOLD);

        //subTitle = "subTitle";

        subTitleview = findViewById(R.id.subTitle);
        subTitleview.setText(subTitle);
        subTitleview.setTextSize(subTitleSize);
        subTitleview.setTextColor(subTitleColor);
        subTitleview.setTypeface(Typeface.DEFAULT_BOLD);
        subTitleview.setVisibility(subTitleVisible?VISIBLE:GONE);

        a.recycle();


        //mTextPaint.setTextSize(mExampleDimension);
        //mTextPaint.setColor(mExampleColor);
        //mTextWidth = mTextPaint.measureText(mExampleString);
    }
    public TextView getTitleView()
    {
        return Titleview;
    }
    public TextView getsubTitleView()
    {
        return subTitleview;
    }
    public ImageView getIconView()
    {
        return Icon;
    }
    public void setTitle(String text)
    {
        Title = text;
        Titleview.setText(text);
    }
    public void setsubTitle(String text)
    {
        subTitle = text;
        subTitleview.setText(text);
    }
    public void setTextColor(int color)
    {
        this.TextColor = color;
        Titleview.setTextColor(color);
    }
    public void setSubTitleColor(int color)
    {
        subTitleColor = color;
        Titleview.setTextColor(color);
    }
    public void setBackgroundColor(int color) {
        Background = findViewById(R.id.Constraint);
        Drawable backRound = getResources().getDrawable(roundimage_array[orientation]);
        backRound.setColorFilter(color, PorterDuff.Mode.SRC_ATOP);
        Background.setBackground(backRound);
    }
}