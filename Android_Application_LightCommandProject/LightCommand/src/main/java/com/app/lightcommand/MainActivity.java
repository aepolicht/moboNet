package com.app.lightcommand;

import java.io.IOException;
import java.util.Locale;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.DatagramPacket;
import java.net.*;
import java.util.Date;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.StrictMode;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.PrintStreamPrinter;
import android.view.Display;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.TimePicker;
import android.widget.ToggleButton;
import android.os.AsyncTask;
import android.view.MotionEvent;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class MainActivity extends FragmentActivity  {

    /**
     * The {@link android.support.v4.view.PagerAdapter} that will provide
     * fragments for each of the sections. We use a
     * {@link android.support.v4.app.FragmentPagerAdapter} derivative, which
     * will keep every loaded fragment in memory. If this becomes too memory
     * intensive, it may be best to switch to a
     * {@link android.support.v4.app.FragmentStatePagerAdapter}.
     */
    SectionsPagerAdapter mSectionsPagerAdapter;

    private int mCurrentItem = 0;
    int modCode = 1, red = 0, green = 0, blue = 0, inten = 0, flash = 1, xPos = 0, yPos = 0;

    /**
     * The {@link ViewPager} that will host the section contents.
     */
    ViewPager mViewPager;

    public void radioChange(View myview) {

        RadioGroup g = (RadioGroup) findViewById(R.id.radioGroup);

        switch (g.getCheckedRadioButtonId())
        {
            case R.id.radioButton:
                modCode = 1;
                break;
            case R.id.radioButton2:
                modCode = 2;
                break;
            case R.id.radioButton3:
                modCode = 3;
                break;
            case R.id.radioButton4:
                modCode = 4;
                break;
            case R.id.radioButton5:
                modCode = 5;
                break;
            case R.id.radioButton6:
                modCode = 6;
                break;
        }

        sendUDP1();
    }

    public void clickA(View myview) {
        ToggleButton t = (ToggleButton)findViewById(R.id.toggleButton);
        sendUDP2( 1, t.isChecked() );
    }

    public void clickB(View myview) {
        ToggleButton t = (ToggleButton)findViewById(R.id.toggleButton2);
        sendUDP2( 2, t.isChecked() );
    }

    public void clickC(View myview) {
        ToggleButton t = (ToggleButton)findViewById(R.id.toggleButton3);
        sendUDP2( 3, t.isChecked() );
    }

    public void clickD(View myview) {
        ToggleButton t = (ToggleButton)findViewById(R.id.toggleButton4);
        sendUDP2( 4, t.isChecked() );
    }

    public void colorClick1(View myview) {
        red   = 255;
        green = 255;
        blue  = 255;

        sendUDP1();
    }
    public void colorClick2(View myview) {
        red   = 0;
        green = 0;
        blue  = 0;

        sendUDP1();
    }
    public void colorClick3(View myview) {
        red   = 255;
        green = 0;
        blue  = 0;

        sendUDP1();
    }
    public void colorClick4(View myview) {
        red   = 195;
        green = 90;
        blue  = 0;

        sendUDP1();
    }
    public void colorClick5(View myview) {
        red   = 240;
        green = 255;
        blue  = 0;

        sendUDP1();
    }
    public void colorClick6(View myview) {
        red   = 0;
        green = 255;
        blue  = 0;

        sendUDP1();
    }
    public void colorClick7(View myview) {
        red   = 0;
        green = 255;
        blue  = 255;

        sendUDP1();
    }
    public void colorClick8(View myview) {
        red   = 0;
        green = 0;
        blue  = 255;

        sendUDP1();
    }
    public void colorClick9(View myview) {
        red   = 150;
        green = 0;
        blue  = 255;

        sendUDP1();
    }
    public void colorClick10(View myview) {
        red   = 200;
        green = 0;
        blue  = 255;

        sendUDP1();
    }

    public void timepickClick(View myview) {
        TimePicker myTime = (TimePicker) findViewById(R.id.timePicker);
        int hour = myTime.getCurrentHour();
        int minute = myTime.getCurrentMinute();
        byte day = 0x00;

        ToggleButton[] days = {
            (ToggleButton) findViewById(R.id.Saturday),
            (ToggleButton) findViewById(R.id.Monday),
            (ToggleButton) findViewById(R.id.Tuesday),
            (ToggleButton) findViewById(R.id.Wednesday),
            (ToggleButton) findViewById(R.id.Thursday),
            (ToggleButton) findViewById(R.id.Friday),
            (ToggleButton) findViewById(R.id.Saturday),
        };

        for (int i = 0; i<7; i++)
            if(days[i].isChecked())
                day = (byte) ( day | ( 1 << i ) );

        sendUDP3(hour, minute, day);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);

        // Create the adapter that will return a fragment for each of the three
        // primary sections of the app.
        mSectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());

        // Set up the ViewPager with the sections adapter.
        mViewPager = (ViewPager) findViewById(R.id.pager);
        mViewPager.setAdapter(mSectionsPagerAdapter);
        mViewPager.setCurrentItem(1);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    public void sendUDP1() {
        byte[] buffer = new byte[8];

        buffer[0] = (byte) 161;
        buffer[1] = (byte) 0;
        buffer[2] = (byte) modCode;
        buffer[3] = (byte) red;
        buffer[4] = (byte) green;
        buffer[5] = (byte) blue;
        buffer[6] = (byte) inten;
        buffer[7] = (byte) flash;

        try {
            InetAddress arduino = InetAddress.getByName("192.168.1.177");
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length, arduino, 8888);
            DatagramSocket socket = new DatagramSocket();
            socket.send(packet);
            socket.close();
        } catch (UnknownHostException e) {
        } catch (IOException e) {
        }

        for (byte b: buffer){
            Log.i("myactivity", String.format("0x%20x", b));
        }
    }

    public void sendUDP2(int num, boolean val) {
        byte[] buffer = new byte[3];

        if (val)
            buffer[2] = (byte) 1;
        else
            buffer[2] = (byte) 0;

        buffer[0] = (byte) 162;
        buffer[1] = (byte) num;

        try {
            InetAddress arduino = InetAddress.getByName("192.168.1.177");
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length, arduino, 8888);
            DatagramSocket socket = new DatagramSocket();
            socket.send(packet);
            socket.close();
        } catch (UnknownHostException e) {
        } catch (IOException e) {
        }
    }

    public void sendUDP3(int hour, int minute, byte days) {
        byte[] buffer = new byte[4];

        buffer[0] = (byte) 163;
        buffer[1] = (byte) hour;
        buffer[2] = (byte) minute;
        buffer[3] = days;

        try {
            InetAddress arduino = InetAddress.getByName("192.168.1.177");
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length, arduino, 8888);
            DatagramSocket socket = new DatagramSocket();
            socket.send(packet);
            socket.close();
        } catch (UnknownHostException e) {
        } catch (IOException e) {
        }
        for (byte b: buffer){
            Log.i("myactivity", String.format("0x%20x", b));
        }
    }

    /**
     * A {@link FragmentPagerAdapter} that returns a fragment corresponding to
     * one of the sections/tabs/pages.
     */
    public class SectionsPagerAdapter extends FragmentPagerAdapter {

        public SectionsPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        @Override
        public Fragment getItem(int position) {
            // getItem is called to instantiate the fragment for the given page.
            // Return a DummySectionFragment (defined as a static inner class
            // below) with the page number as its lone argument.
            Fragment fragment;

            if (position == 1)
            {
                fragment = new p1Fragment();
            } else if (position == 0)
            {
                fragment = new p2Fragment();
            } else
            {
                fragment = new p3Fragment();
            }
            return fragment;
        }

        @Override
        public int getCount() {
            // Show 3 total pages.
            return 3;
        }

        @Override
        public CharSequence getPageTitle(int position) {
            Locale l = Locale.getDefault();
            switch (position) {
                case 1:
                    return "Controls";
                case 0:
                    return "Color Selector";
                case 2:
                    return "Alarm";
            }
            return null;
        }
    }

    /**
     * A dummy fragment representing a section of the app, but that simply
     * displays dummy text.
     */
    public class p1Fragment extends Fragment implements OnSeekBarChangeListener {
        /**
         * The fragment argument representing the section number for this
         * fragment.
         */
        public p1Fragment() {
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.page_1, container, false);

            final SeekBar seekbar1 = (SeekBar) rootView.findViewById(R.id.seekBar);
            seekbar1.setOnSeekBarChangeListener(this);
            final SeekBar seekbar2 = (SeekBar) rootView.findViewById(R.id.seekBar2);
            seekbar2.setOnSeekBarChangeListener(this);

            return rootView;
        }

        @Override
        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            if(seekBar.getId() == R.id.seekBar)
                inten = progress;
            else
                flash = progress;
            sendUDP1();
        }

        @Override
        public void onStartTrackingTouch(SeekBar arg0) {
            // TODO Auto-generated method stub
        }

        @Override
        public void onStopTrackingTouch(SeekBar arg0) {
            // TODO Auto-generated method stub
        }
    }
    public class p2Fragment extends Fragment {
        /**
         * The fragment argument representing the section number for this
         * fragment.
         */

        public p2Fragment() {
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {

            View rootView = inflater.inflate(R.layout.page_2, container, false);

            if (rootView != null) {
                rootView.setOnTouchListener(new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View view, MotionEvent e) {
                        xPos = (int) e.getX();
                        yPos = (int) e.getY();
                        Log.d("Touch"," "+xPos+" "+yPos);

                        int viewWidth = view.getWidth();
                        int viewHeight = view.getHeight();


                        Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.spectrum);
                        Bitmap resizedBitmap = Bitmap.createScaledBitmap(bitmap,viewWidth, viewHeight, false);


                        if( (xPos >= 0) && (xPos < viewWidth) && (yPos >= 0) && (yPos < viewHeight) )
                        {
                            int pixel = resizedBitmap.getPixel(xPos,yPos);
                            red = Color.red(pixel);
                            blue = Color.blue(pixel);
                            green = Color.green(pixel);
                        }

                        Log.d("Colors","R= "+red+" G= "+green+" B= "+blue);
                        sendUDP1();
                        return false;
                    }
                });
            }

            return rootView;
        }
    }
    public class p3Fragment extends Fragment {
        /**
         * The fragment argument representing the section number for this
         * fragment.
         */
        public p3Fragment() {
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.page_3, container, false);
            return rootView;
        }
    }

    boolean GetBit(byte b, int bitNumber)
    {
        return (b & (1 << bitNumber)) != 0;
    }
}