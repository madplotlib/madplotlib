/* Copyright (C) 2017 Karl Phillip Buhr <karlphillip@gmail.com>
 *
 * This work is licensed under the MIT License.
 * To view a copy of this license, visit:
 *      https://opensource.org/licenses/MIT
 *
 * This file is part of Madplotlib, a C++ library for building simple
 * 2D plots inspired on matplotlib.
 */
/*
 * plot() has many forms. This file demonstrates how to use some of them.
 * However, the most complete form of plot() currently is:
 *     plot(x, y, marker, label, alpha, color, linewidth, markeredgecolor, markersize)
 *
 * Happy plotting!
 */

#include <Eigen/Dense>

#include "Madplotlib.h"

#include <QApplication>

// Uncomment the line below to save each chart as PNG image
#define SCRSHOT

/* Use case: simple line chart.
 * plot() draws the values of x and y on a line chart.
 * show() creates a new window to display the chart.
 * savefig() saves a screenshot of the chart as PNG image.
 */
void test1()
{
    Eigen::ArrayXf x(16);
    x <<   0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7,
         0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5;

    Eigen::ArrayXf y(16);
    y <<  65,  79,  80,  68,  77,  81, 100, 102,
         105, 111, 120, 126, 120, 104,  85,  92;

    Madplotlib plt;
    plt.plot(x, y);
    plt.show();

#ifdef SCRSHOT
    plt.savefig("test1.png");
#endif
}

/* Use case: simple scatter chart.
 * plot() draws the values of x and y on a scatter chart.
 * show() creates a new window to display the chart.
 */
void test2()
{
    float period = 0.5;
    Eigen::ArrayXf x = Eigen::ArrayXf::LinSpaced(72, 0, 20);
    Eigen::ArrayXf y = x.cos() * period;

    Madplotlib plt;
    plt.title("Test 2: Simple Scatter Plot");
    plt.plot(x, y, QString("o"));
    plt.show();

#ifdef SCRSHOT
    plt.savefig("test2.png");
#endif
}

/* Use case: plot 3 data sets on a line chart.
 * + axis() defines the X,Y range to make the labels look cooler.
 * + plot() draws X and Y as a continuous line, thicker (linewidth = 4).
 * + plot() adds 5 to every element if the Y axis and then draws it as a dashed line.
 * + plot() adds 10 to every element if the Y axis and then draws it as a dotted line.
 * + legend() with no args, displays the labels passed to plot() at a default position.
 */
void test3()
{
    float period = 2;
    Eigen::ArrayXf x = Eigen::ArrayXf::LinSpaced(100, 0, 25);
    Eigen::ArrayXf y = x.cos() * period;

    Madplotlib plt;
    plt.title("Test 3: Multiple Data Series");
    plt.axis(0, 25, 0, 14);
    plt.plot(x, y, marker=QString("--"), label=QString("label=Dashed Line"));
    plt.plot(x, y+5, label=QString("label=Default Line"), (quint32)4);
    plt.plot(x, y+10, QString("."), label=QString("label=Dotted Line"));
    plt.legend(); // default position is "lower center"
    plt.show();

#ifdef SCRSHOT
    plt.savefig("test3.png");
#endif
}

/* Use case: simple scatter chart.
 * + Generates 50 random values between [0, 50] for the first data series.
 * + Generates 72 random values between [0, 75] for the second data series.
 * + title()
 * + locator_params() displays 10 ticks on the X axis.
 * + axis() defines the range of values for each axis.
 * + plot() uses the parameter "o" for a scatter plot,
 *          changes the transparency level to 30% for a red color marker,
 *          with a markersize of 8.
 * + plot() uses the parameter "o" for a scatter plot,
 *          changes the transparency level to 50% for a blue color marker.
 */
void test4()
{
    Eigen::ArrayXf x_rand = Eigen::ArrayXf::Random(64) * 50;
    Eigen::ArrayXf x = x_rand.abs();

    Eigen::ArrayXf y_rand = Eigen::ArrayXf::Random(64) * 50;
    Eigen::ArrayXf y = y_rand.abs();

    Eigen::ArrayXf x_rand2 = Eigen::ArrayXf::Random(72) * 75;
    Eigen::ArrayXf x2 = x_rand2.abs();

    Eigen::ArrayXf y_rand2 = Eigen::ArrayXf::Random(72) * 75;
    Eigen::ArrayXf y2 = y_rand2.abs();

    Madplotlib plt;
    plt.title("Test 4: Random Scatter Plot");
    plt.locator_params("x", 10);
    plt.axis(-25, 100, -25, 100);
    plt.plot(x, y, marker=QString("o"), 0.7f, QColor(255, 0, 0), 8.0f); // red, 30% transparent, markersize 8
    plt.plot(x2, y2, marker=QString("o"), 0.5f, QColor(0, 0, 255));     // blue, 50% transparent
    plt.show();

#ifdef SCRSHOT
    plt.savefig("test4.png");
#endif
}


/* Use case that defines 3 series of data containing only Y axis values.
 * + title()
 * + ylabel()
 * + xlabel()
 * + plot() automatically creates data for the X axis and labels it Linear.
 * + plot() automatically creates data for the X axis and labels it Exponential.
 * + plot() automatically creates data for the X axis and labels it Flat.
 * + legend() displays all labels set through plot() in a legend positioned at the right.
 */
void test5()
{
    // linear
    Eigen::ArrayXf a = Eigen::ArrayXf::LinSpaced(20, 0, 2000);

    // exponential
    Eigen::ArrayXf b = Eigen::ArrayXf::LinSpaced(20, 0, 100);
    b = b * b;

    // flat
    Eigen::ArrayXf c = Eigen::ArrayXf::Zero(20);
    c = 1000;

    Madplotlib plt;
    plt.title("Test 5: Linear vs Exponential vs Flat");
    plt.ylabel("Y Values");
    plt.xlabel("X Values");

    plt.plot(a, QString("label=Linear"));
    plt.plot(b, QString("label=Exponential"));
    plt.plot(c, QString("label=Flat"));
    plt.legend("loc=center right");
    plt.show();

#ifdef SCRSHOT
    plt.savefig("test5.png");
#endif
}


/* Use case that plots 4 data sets.
 * + plot() argument "o" defines a scatter plot.
 * + plot() argument QColor defines the color for the line plot.
 * + xlabel() defines the label for the X axis.
 * + ylabel() defines the label for the Y axis.
 */
void test6()
{
    Eigen::ArrayXf x(16);
    x <<   0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7,
         0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5;

    Eigen::ArrayXf y(16);
    y <<  65,  79,  80,  68,  77,  81, 100, 102,
         105, 111, 120, 126, 120, 104,  85,  92;

    Madplotlib plt;
    plt.plot(x, y, color=QColor(0xFF2700));               // red
    plt.plot(x, y, marker=QString("o"), color=QColor(0xFF2700));
    plt.plot(x, y-40, color=QColor(0x008FD5));            // blue
    plt.plot(x, y-40, marker=QString("o"), color=QColor(0x008FD5));

    plt.title("Test 6: Line + Scatter");
    plt.xlabel("X values");
    plt.ylabel("Y values");
    plt.show();

#ifdef SCRSHOT
    plt.savefig("test6.png");
#endif
}

/* Use case with several UI customizations.
 * + title()
 * + xlabel() defines the label for the X axis.
 * + ylabel() defines the label for the Y axis.
 * + yticks() changes the default categories of Y axis for customized ones:
 *      Low:    represents all data between [0, 1]
 *      High:   represents all data between [1, 2]
 * + ylim() defines the range for the Y axis.
 * + plot() draws a scatter plot with larger circles (markersize = 7.0).
 * + grid() enables drawing of the background grid.
 */
void test7()
{
    const double pi = std::acos(-1.0);

    Eigen::ArrayXf x = Eigen::ArrayXf::LinSpaced(64, 0.0, 5.0);
    Eigen::ArrayXf y = Eigen::cos(2 * pi * x) * Eigen::exp(-x);

    Eigen::ArrayXf y_ticks(2);
    y_ticks << 1, 2;

    QVector<QString> y_labels;
    y_labels.push_back("Low");
    y_labels.push_back("High");

    Madplotlib plt;
    plt.title("Test 7: UI Customizations: Grid & Categories");
    plt.xlabel("time (s)");
    plt.ylabel("voltage (mV)");
    plt.yticks(y_ticks, y_labels);

    // On Qt 7.5, Qt Charts has a bug spacing correctly categories on negative Y axis (-1, 1).
    // For now, drawing (y+1) will bypass that since the values will fall between (0, 2).
    plt.ylim(0, 2);
    plt.plot(x, y+1, marker=QString("o"), alpha=1.f, linewidth=2, markersize=7.0f); // alpha=1.f, linewidth=2, markersize=7.f
    plt.grid(true);

    plt.show();

#ifdef SCRSHOT
    plt.savefig("test7.png");
#endif
}

/* Use case that displays two waves on a scatter plot and then connects them with lines.
 * + axis() param "off" hides both axis ticks.
 * + plot() draw the X series using x's squared root as the Y series.
 * + plot() draw the X series using x's negative squared root as the Y series.
 * + noise is an array with 50 random numbers between [0, 2].
 * + plot() draw the X series using the difference of x's squared root and the noise asthe Y series.
 * + plot() draw the X series using the difference of x's negative squared root and the noise as the Y series.
 */
void test8()
{
    Eigen::ArrayXf x = Eigen::ArrayXf::LinSpaced(50, 0, 100);

    Madplotlib plt;
    plt.title("Test 8: Line + Square Markers + Hidden Ticks");
    plt.axis("off");
    plt.plot(x, x.sqrt(), color=QColor(0, 0, 0));
    plt.plot(x, -x.sqrt(), color = QColor(0, 0, 0));

    Eigen::ArrayXf noise = Eigen::ArrayXf::Random(50) * 2;

    plt.plot(x, x.sqrt() - noise, marker=QString("s"), alpha = 0.7f, color=QColor(19, 154, 255), edgecolor=QColor(19, 154, 255)); // red squares without black edges
    plt.plot(x, -x.sqrt() - noise, marker = QString("s"), alpha = 0.7f, color=QColor(255, 41, 5), edgecolor=QColor(255, 41, 5));    // blue squares without black edges
    plt.show();

#ifdef SCRSHOT
    plt.savefig("test8.png");
#endif
}

/* Use case that displays Y data twice with custom limits and ticks for both axis.
 * + title()
 * + xlabel()
 * + ylabel()
 * + plot() draws X,Y using a specific color.
 * + plot() uses circular markers with green edges filled with white.
 */
void test9()
{
    Eigen::ArrayXf x(26);
    x << 0, 102, 205, 301, 404,
         500, 601, 701, 804, 904,
         1006, 1503, 2016, 3086, 4002,
         5178, 10094, 16012, 21267, 25985,
         32641, 48486, 54925, 58145, 63291,
         98029;

    Eigen::ArrayXf y(26);
    y << 924, 794, 2708, 3324, 5037,
         3849, 6150, 5975, 9275, 5410,
         9222, 10592, 12374, 22348, 27508,
         18361, 39910, 31354, 36074, 20413,
         69383, 252988, 12457, 48495, 171303,
         69783;

    Madplotlib plt;
    plt.title("Test 9: Fox News Facebook Shares vs Likes");
    plt.xlabel("Fox News Shares");
    plt.ylabel("Fox News Likes");

    plt.plot(x, y, QColor(169, 206, 0)); // plot green line
    plt.plot(x, y, QString("o"), QColor(255, 255, 255), 2, QColor(169, 206, 0), 6.5f); // plot markers, linewith=2, markersize=6.5
    plt.show();

#ifdef SCRSHOT
    plt.savefig("test9.png");
#endif
}

/* Use case that displays Y data twice and hides the labels from X axis.
 * + plot() draws Y data as a thicker red line (linewidth = 4).
 * + plot() draws Y data as scattered with larger colored markers (markersize = 15.0f).
 */
void test10()
{
    const double pi = std::acos(-1.0);
    Eigen::ArrayXf X = Eigen::ArrayXf::LinSpaced(256, 0, 2*pi);
    Eigen::ArrayXf C = Eigen::cos(X);
    Eigen::ArrayXf S = Eigen::sin(X);

    Madplotlib plt;
    plt.title("Test 10");
    plt.grid(true);

    Eigen::ArrayXf x_ticks(4);
    x_ticks << pi/2, pi, (3*pi)/2, 2*pi;

    QVector<QString> x_labels;
    x_labels.push_back("π/2");
    x_labels.push_back("π");
    x_labels.push_back("3π/2");
    x_labels.push_back("2π");

    // On Qt 7.5, Qt Charts has a bug spacing correctly categories on negative X axis (-3.1, 3.1).
    // For now, make sure you are using only positive X values to bypass that problem.
    plt.plot(X, C);
    plt.plot(X, S, marker=QString("--"));
    plt.xlim(0, 2*pi);
    plt.xticks(x_ticks, x_labels);
    plt.show();

#ifdef SCRSHOT
    plt.savefig("test10.png");
#endif
}

void run_test(int id)
{
    if (id == 0 || id == 1)
        test1();

    if (id == 0 || id == 2)
        test2();

    if (id == 0 || id == 3)
        test3();

    if (id == 0 || id == 4)
        test4();

    if (id == 0 || id == 5)
        test5();

    if (id == 0 || id == 6)
        test6();

    if (id == 0 || id == 7)
        test7();

    if (id == 0 || id == 8)
        test8();

    if (id == 0 || id == 9)
        test9();

    if (id == 0 || id == 10)
        test10();
}

void run_test(int begin, int end)
{
    for (int i = begin; i <= end; i++)
        run_test(i);
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // runtest(0)    - executes all tests.
    // runtest(5, 9) - executes all tests between 5 and 9.
    run_test(0);

    qInfo() << "* Done!";

    // NOTE:
    // Creating an object with the default constructor makes the window
    // block/freeze your program execution upon show().
    // When the user closes the window, your program continues to run
    // normally until the next show() is called.
    // This is how matplotlib behaves!
    //
    // However, if you don't want Madplotlib to block you need
    // to create a Madplotlib object and pass TRUE to the constructor.
    // This will make Madplotlib behave like a traditional widget.
    // This is what you need to do if you plan to write custom Qt GUIs
    // with widgets. Also, don't forget to invoke app.exec().

    return 0;
}


