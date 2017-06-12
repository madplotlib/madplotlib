/* Copyright (C) 2017 Karl Phillip Buhr <karlphillip@gmail.com>
 *
 * This work is licensed under the MIT License.
 * To view a copy of this license, visit:
 *      https://opensource.org/licenses/MIT
 *
 * This file is part of Madplotlib, a C++ library for building simple
 * 2D plots inspired on matplotlib.
 */
#pragma once
#ifndef NO_EIGEN
#include <Eigen/Dense>
#endif

#include <QDebug>
#include <QEventLoop>
#include <QGraphicsLayout>
#include <QPair>

#include <QtCharts/QCategoryAxis>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>


/* Debug control */

#define DEBUG               0      // level 0: debug msgs are disabled
                                   // level 1: print method calls
                                   // level 2: print method calls + data

/* Global definitions */

#define SHOW_TICK           1
#define HIDE_TICK           2
#define SHOW_CUSTOM_TICK    4

#define DEFAULT_LEGEND      ""
#define DEFAULT_MARKER      "-"
#define DEFAULT_ALPHA       1.0f
#define DEFAULT_COLOR       "none"
#define DEFAULT_EDGECOLOR   "none"
#define DEFAULT_LINEW       2
#define DEFAULT_MARKERSZ    6.0f

#ifdef NO_EIGEN
#error COMPILATION MUST GO THOUGH WITHOUT EIGEN CODE.
#endif


class Madplotlib
{
public:
    Madplotlib(bool isWidget = false)
    : _chart(NULL), _chartView(NULL), _isWidget(isWidget)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "Madplotlib(): isWidget=" << isWidget;
#endif
        _chart = new QtCharts::QChart();
        _chartView = new QtCharts::QChartView(_chart);

        _enableGrid = false;
        _customLimits = false;
        _xMin = _xMax = _yMin = _yMax = 0;

        _showXticks = _showYticks = SHOW_TICK;
        _xTickCount = 7;
        _yTickCount = 5;

        _colorIdx = 0;
        _colors.push_back(QColor(0x1f77b4));
        _colors.push_back(QColor(0xff7f0e));
        _colors.push_back(QColor(0x2ca02c));
        _colors.push_back(QColor(0xd62728));
        _colors.push_back(QColor(0x9467bd));
        _colors.push_back(QColor(0x8c564b));
        _colors.push_back(QColor(0xe377c2));
        _colors.push_back(QColor(0x7f7f7f));
        _colors.push_back(QColor(0xbcbd22));
        _colors.push_back(QColor(0x17becf));
    }

    void axis(QString cmd)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "axis(): cmd=" << cmd;
#endif
        if (cmd == "off")
        {
            _showYticks = _showXticks = HIDE_TICK;
        }
        else if (cmd == "xoff")
        {
            _showXticks = HIDE_TICK;
        }
        else if (cmd == "yoff")
        {
            _showYticks = HIDE_TICK;
        }
        else
        {
            qCritical() << "axis()!!! options are 'off', 'xoff' and 'yoff'.";
            return;
        }
    }

    /* axis(): gets the current axes limits [xMin, xMax, yMin, yMax].
     */
    void axis(qreal* xMin, qreal* xMax, qreal* yMin, qreal* yMax)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "axis(): _xMin=" << _xMin << " _xMax=" << _xMax << " _yMin=" << _yMin << " _yMax=" << _yMax;
#endif
        *xMin = _xMin;
        *xMax = _xMax;
        *yMin = _yMin;
        *yMax = _yMax;
    }

    /* axis(): sets the viewport of the axis by a list of [xMin, xMax, yMin, yMax].
     */
    void axis(const qreal& xMin, qreal xMax, const qreal& yMin, const qreal& yMax)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "axis(): xMin=" << xMin << " xMax=" << xMax << " yMin=" << yMin << " yMax=" << yMax;
#endif
        _xMin = xMin;
        _xMax = xMax;
        _yMin = yMin;
        _yMax = yMax;
        _customLimits = true;
    }

    /* xlim(): sets the x limits of the current axes.
     */
    void xlim(const qreal& xMin, const qreal& xMax)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "xlim(): xMin=" << xMin << " xMax=" << xMax;
#endif
        _xMin = xMin;
        _xMax = xMax;
        _customLimits = true;
    }

    /* ylim(): sets the x limits of the current axes.
     */
    void ylim(const qreal& yMin, const qreal& yMax)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "ylim(): yMin=" << yMin << " yMax=" << yMax;
#endif
        _yMin = yMin;
        _yMax = yMax;
        _customLimits = true;
    }

    /* title(): defines the title of the chart.
     */
    void title(QString string)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "title(): string=" << string;
#endif
        _title = string;
    }

    /* xlabel(): defines the label displayed below the x axis.
     */
    void xlabel(QString label)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "xlabel(): label=" << label;
#endif
        _xLabel = label;
    }

    /* ylabel(): defines the label displayed to the left of the y axis.
     */
    void ylabel(QString label)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "ylabel(): label=" << label;
#endif
        _yLabel = label;
    }

    /* legend(): defines the position of the legend label inside the chart.
     */
    void legend()
    {
        _legendPos = "lower center";
    }

    /* legend(): defines the position of the legend label inside the chart.
     */
    void legend(QString cmd)
    {
        _legendPos = _parseLegendPos(cmd);
    }

    /* grid(): enables or disables the background grid of the chart.
     */
    void grid(bool status)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "grid(): status=" << status;
#endif
        _enableGrid = status;
    }

    /* savefig(): saves the chart displayed by show() as an image on the disk.
     */
    void savefig(QString filename)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "savefig(): filename=" << filename;
#endif
        if (!_pixmap.isNull())
            _pixmap.save(filename);
    }

    /* xticks(): sets the x-limits of the current tick locations and labels.
     */
    void xticks(const Eigen::ArrayXf& values, const QVector<QString>& labels)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "xticks(): values.sz=" << values.rows() << " labels.sz=" << labels.size();
#endif
        if (values.rows() == 0 && labels.size() == 0)
        {
            _showXticks = HIDE_TICK;
            return;
        }

        if (values.rows() != labels.size())
        {
            qCritical() << "xticks(): the amount of values and labels must match!";
            return;
        }

        for (int i = 0; i < values.rows(); i++)
            _xTicks.push_back(QPair<QString, qreal>(labels[i], values[i]));

        _showXticks = SHOW_CUSTOM_TICK;

#if (DEBUG > 1) && (DEBUG < 3)
        qDebug() << "xticks(): xticks.sz=" << _xTicks.size();
        for (int i = 0; i < _xTicks.size(); i++)
            qDebug() << "\t" << _xTicks[i].second << " = " << _xTicks[i].first;
#endif
    }

    /* yticks(): sets the y-limits of the current tick locations and labels.
     */
    void yticks(const Eigen::ArrayXf& values, const QVector<QString>& labels)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "yticks(): values.sz=" << values.rows() << " labels.sz=" << labels.size();
#endif
        if (values.rows() == 0 && labels.size() == 0)
        {
            _showYticks = HIDE_TICK;
            return;
        }

        if (values.rows() != labels.size())
        {
            qCritical() << "PlotLib::xticks(): the amount of values and labels must match!" ;
            return;
        }

        for (int i = 0; i < values.rows(); i++)
            _yTicks.push_back(QPair<QString, qreal>(labels[i], values[i]));

        _showYticks = SHOW_CUSTOM_TICK;

#if (DEBUG > 1) && (DEBUG < 3)
        qDebug() << "yticks(): yticks.sz=" << _yTicks.size();
        for (int i = 0; i < _yTicks.size(); i++)
            qDebug() << "\t" << _yTicks[i].second << " = " << _yTicks[i].first;
#endif
    }

    /* locator_params(): reduce or increase the amount of ticks for each axis.
     */
    void locator_params(QString axis, int nbins)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "locator_params(): axis=" << axis << " nbins=" << nbins;
#endif
        if (axis == "x")
        {
            _xTickCount = nbins;
        }
        else if (axis == "y")
        {
            _yTickCount = nbins;
        }
        else if (axis == "both")
        {
            _yTickCount = _xTickCount = nbins;
        }
        else
        {
            qCritical() << "locator_params(): '" << axis << "' is not a valid option.";
            return;
        }
    }

    /* Overloaded plot(y) methods with 1-2 parameters */

    void plot(const Eigen::ArrayXf& y)
    {
        plot(y, DEFAULT_MARKER, DEFAULT_LEGEND, DEFAULT_ALPHA, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const qreal& alpha)
    {
        plot(y, DEFAULT_MARKER, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QColor& color)
    {
        plot(y, DEFAULT_MARKER, DEFAULT_LEGEND, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const quint32& linewidth)
    {
        plot(y, DEFAULT_MARKER, DEFAULT_LEGEND, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    /* Overloaded plot(y) methods with 3 parameters */

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, DEFAULT_ALPHA, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);

    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const quint32& linewidth)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const qreal& alpha, const QColor& color)
    {
        plot(y, DEFAULT_MARKER, DEFAULT_LEGEND, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const qreal& alpha, const quint32& linewidth)
    {
        plot(y, DEFAULT_MARKER, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const qreal& alpha, const qreal& markersize)
    {
        plot(y, DEFAULT_MARKER, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QColor& color, const quint32& linewidth)
    {
        plot(y, DEFAULT_MARKER, DEFAULT_LEGEND, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QColor& color, const qreal& markersize)
    {
        plot(y, DEFAULT_MARKER, DEFAULT_LEGEND, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    /* Overloaded plot(y) methods with 4 parameters */

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const quint32& linewidth)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const quint32& linewidth)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color, const QColor& edgecolor)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color, const quint32& linewidth)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const quint32& linewidth, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const quint32& linewidth, const QColor& edgecolor)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const qreal& alpha, const QColor& color, const quint32& linewidth)
    {
        plot(y, DEFAULT_MARKER, DEFAULT_LEGEND, alpha, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    /* Overloaded plot(y) methods with 5 parameters */

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const quint32& linewidth)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, cmd2, cmd1, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const QColor& edgecolor)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const quint32& linewidth)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, cmd2, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const quint32& linewidth, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, cmd2, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const quint32& linewidth, const QColor& edgecolor)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color, const QColor& edgecolor)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color, const quint32& linewidth)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const quint32& linewidth, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const quint32& linewidth, const QColor& edgecolor)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, linewidth, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, DEFAULT_COLOR, linewidth, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color, const QColor& edgecolor, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, markersize);
        else
            plot(y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color, const quint32& linewidth, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    /* Overloaded plot(y) methods with 6 parameters */

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color, const QColor& edgecolor)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, alpha, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, alpha, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color, const quint32& linewidth)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, alpha, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, alpha, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, cmd2, cmd1, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const quint32& linewidth, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, cmd2, cmd1, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const quint32& linewidth, const QColor& edgecolor)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, DEFAULT_ALPHA, color, linewidth, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, DEFAULT_ALPHA, color, linewidth, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const QColor& edgecolor, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, markersize);
        else
            plot(y, cmd2, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const quint32& linewidth, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, cmd2, cmd1, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1,const qreal& alpha, const QColor& color, const quint32& linewidth, const QColor& edgecolor)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, color, linewidth, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, color, linewidth, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color, const QColor& edgecolor, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, color, DEFAULT_LINEW, edgecolor, markersize);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, color, DEFAULT_LINEW, edgecolor, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color, const quint32& linewidth, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, color, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, color, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color, const quint32& linewidth, const QColor& edgecolor, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, linewidth, edgecolor, markersize);
        else
            plot(y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, linewidth, edgecolor, markersize);
    }

    /* Overloaded plot(y) methods with 7 parameters */

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color, const quint32& linewidth, const QColor& edgecolor)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, alpha, color, linewidth, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(y, cmd2, cmd1, alpha, color, linewidth, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color, const QColor& edgecolor, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, alpha, color, DEFAULT_LINEW, edgecolor, markersize);
        else
            plot(y, cmd2, cmd1, alpha, color, DEFAULT_LINEW, edgecolor, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color, const quint32& linewidth, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, cmd2, alpha, color, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(y, cmd2, cmd1, alpha, color, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const quint32& linewidth, const QColor& edgecolor, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, linewidth, edgecolor, markersize);
        else
            plot(y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, linewidth, edgecolor, markersize);
    }

    void plot(const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color, const quint32& linewidth, const QColor& edgecolor, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(y, cmd1, DEFAULT_LEGEND, alpha, color, linewidth, edgecolor, markersize);
        else
            plot(y, DEFAULT_MARKER, cmd1, alpha, color, linewidth, edgecolor, markersize);
    }

    /* Overloaded plot(x,y) methods with 2-3 parameters */

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y)
    {
        plot(x, y, DEFAULT_MARKER, DEFAULT_LEGEND, DEFAULT_ALPHA, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const qreal& alpha)
    {
        plot(x, y, DEFAULT_MARKER, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QColor& color)
    {
        plot(x, y, DEFAULT_MARKER, DEFAULT_LEGEND, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const quint32& linewidth)
    {
        plot(x, y, DEFAULT_MARKER, DEFAULT_LEGEND, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    /* Overloaded plot(x,y) methods with 4 parameters */

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, DEFAULT_ALPHA, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);

    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const quint32& linewidth)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const qreal& alpha, const QColor& color)
    {
        plot(x, y, DEFAULT_MARKER, DEFAULT_LEGEND, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const qreal& alpha, const quint32& linewidth)
    {
        plot(x, y, DEFAULT_MARKER, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const qreal& alpha, const qreal& markersize)
    {
        plot(x, y, DEFAULT_MARKER, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QColor& color, const quint32& linewidth)
    {
        plot(x, y, DEFAULT_MARKER, DEFAULT_LEGEND, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QColor& color, const qreal& markersize)
    {
        plot(x, y, DEFAULT_MARKER, DEFAULT_LEGEND, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    /* Overloaded plot(x,y) methods with 5 parameters */

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const quint32& linewidth)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const quint32& linewidth)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color, const QColor& edgecolor)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color, const quint32& linewidth)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const quint32& linewidth, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const quint32& linewidth, const QColor& edgecolor)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const qreal& alpha, const QColor& color, const quint32& linewidth)
    {
        plot(x, y, DEFAULT_MARKER, DEFAULT_LEGEND, alpha, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    /* Overloaded plot(x,y) methods with 6 parameters */

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const quint32& linewidth)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, cmd2, cmd1, alpha, DEFAULT_COLOR, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const QColor& edgecolor)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const quint32& linewidth)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, cmd2, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const quint32& linewidth, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, cmd2, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const quint32& linewidth, const QColor& edgecolor)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, DEFAULT_ALPHA, DEFAULT_COLOR, linewidth, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color, const QColor& edgecolor)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color, const quint32& linewidth)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const quint32& linewidth, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const quint32& linewidth, const QColor& edgecolor)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, DEFAULT_COLOR, linewidth, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, DEFAULT_COLOR, linewidth, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color, const QColor& edgecolor, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, markersize);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color, const quint32& linewidth, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    /* Overloaded plot(x,y) methods with 7 parameters */

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color, const QColor& edgecolor)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, alpha, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, alpha, color, DEFAULT_LINEW, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color, const quint32& linewidth)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, alpha, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, alpha, color, linewidth, DEFAULT_EDGECOLOR, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, cmd2, cmd1, alpha, color, DEFAULT_LINEW, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const quint32& linewidth, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, cmd2, cmd1, alpha, DEFAULT_COLOR, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const quint32& linewidth, const QColor& edgecolor)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, DEFAULT_ALPHA, color, linewidth, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, DEFAULT_ALPHA, color, linewidth, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const QColor& edgecolor, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, markersize);
        else
            plot(x, y, cmd2, cmd1, DEFAULT_ALPHA, color, DEFAULT_LINEW, edgecolor, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const quint32& linewidth, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, cmd2, cmd1, DEFAULT_ALPHA, color, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1,const qreal& alpha, const QColor& color, const quint32& linewidth, const QColor& edgecolor)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, color, linewidth, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, color, linewidth, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color, const QColor& edgecolor, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, color, DEFAULT_LINEW, edgecolor, markersize);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, color, DEFAULT_LINEW, edgecolor, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color, const quint32& linewidth, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, color, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, color, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QColor& color, const quint32& linewidth, const QColor& edgecolor, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, linewidth, edgecolor, markersize);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, linewidth, edgecolor, markersize);
    }

    /* Overloaded plot(x,y) methods with 8 parameters */

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color, const quint32& linewidth, const QColor& edgecolor)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, alpha, color, linewidth, edgecolor, DEFAULT_MARKERSZ);
        else
            plot(x, y, cmd2, cmd1, alpha, color, linewidth, edgecolor, DEFAULT_MARKERSZ);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color, const QColor& edgecolor, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, alpha, color, DEFAULT_LINEW, edgecolor, markersize);
        else
            plot(x, y, cmd2, cmd1, alpha, color, DEFAULT_LINEW, edgecolor, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const qreal& alpha, const QColor& color, const quint32& linewidth, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, cmd2, alpha, color, linewidth, DEFAULT_EDGECOLOR, markersize);
        else
            plot(x, y, cmd2, cmd1, alpha, color, linewidth, DEFAULT_EDGECOLOR, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const QString& cmd2, const QColor& color, const quint32& linewidth, const QColor& edgecolor, const qreal& markersize)
    {
        _check_cmds_are_good(cmd1, cmd2);

        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, DEFAULT_ALPHA, color, linewidth, edgecolor, markersize);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, DEFAULT_ALPHA, color, linewidth, edgecolor, markersize);
    }

    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const QString& cmd1, const qreal& alpha, const QColor& color, const quint32& linewidth, const QColor& edgecolor, const qreal& markersize)
    {
        if (_is_marker(cmd1))
            plot(x, y, cmd1, DEFAULT_LEGEND, alpha, color, linewidth, edgecolor, markersize);
        else
            plot(x, y, DEFAULT_MARKER, cmd1, alpha, color, linewidth, edgecolor, markersize);
    }

    /* plot(y): this implementation handles calls that do not specify X data.
     * For that purpose, this method fabricates the data needed to plot Y series
     * correctly on the chart.
     */
    void plot(const Eigen::ArrayXf& y, const QString& marker, const QString& label,
              const qreal& alpha, const QColor& color, const quint32& linewidth,
              const QColor& edgecolor, const qreal& markersize)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "plot(y): marker=" << marker << " alpha=" << alpha <<
                    " color=" << color << " edgecolor=" << edgecolor <<
                    " linewidth=" << linewidth << " markersize=" << markersize;
#endif
        if (y.rows() == 0)
        {
            qCritical() << "plot(y): axis size must be > 0 but its " << y.rows();
            exit(-1);
        }

        // number of x values needed to accompany the y values
        int num_items = y.rows();

        // make up X data and plot into series, but take into account that xlim()
        // could have been called with the start and end of x series.
        Eigen::ArrayXf x = Eigen::ArrayXf(y.rows());
        qreal x_inc = 1;
        if (_customLimits && (_xMin != _xMax))
        {
            qreal xrange = _xMax - _xMin;
            x_inc = xrange / y.rows();
        }

        qreal x_value = _xMin;
        for (int i = 0; i < num_items; i++)
        {
            x[i] = x_value;
            x_value += x_inc;

#if (DEBUG > 1) && (DEBUG < 3)
        qDebug() << "plot(y): generated x[" << i << "]=" << x[i];
#endif
        }

        plot(x, y, marker, label, alpha, color, linewidth, edgecolor, markersize);
    }

    /* plot(): called when user needs to put data on a chart.
     * x: an array that stores x axis values.
     * y: an array that stores y axis values.     
     * marker: chart types. "-" for line plot and "o" for scatter plot.
     * alpha: defines the transparency level of the color.
     * color: defines the color used to draw the data.
     * edgecolor: defines the edge color of "o" marker.
     * linewidth: defines the width of the pen used to draw "-" marker.
     * markersize: defines the size of "o" marker.
     */
    void plot(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y,
              const QString& marker, const QString& label,
              const qreal& alpha, const QColor& color, const quint32& linewidth,
              const QColor& edgecolor, const qreal& markersize)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "plot(x,y): marker:" << marker << " alpha:" << alpha <<
                    " color:" << color << " edgecolor:" << edgecolor <<
                    " linewidth:" << linewidth << " markersize:" << markersize;
#endif

        if (marker != "-" && marker != "--" && marker != "." && marker != "o" && marker != "s")
        {
            qCritical() << "plot(x,y): unknown marker '" << marker << "'.";
            return;
        }

        if (x.rows() != y.rows())
        {
            qCritical() << "plot(x,y): x.sz=" << x.cols() << " != y.sz=" << y.rows();
            exit(-1);
        }

        if (x.rows() == 0 || y.rows() == 0)
        {
            qCritical() << "plot(x,y): axis size must be > 0 but it is x.sz=" << x.rows() << " y.sz=" << y.rows();
            exit(-1);
        }

        // Make a copy because it's show() who setup these things
        _legend = label;

        // find min and max values to define the range of the X axis
        qreal xMin = x.minCoeff();
        qreal xMax = x.maxCoeff();
        if (xMin < _xMin)
            _xMin = xMin;
        if (xMax > _xMax)
            _xMax = xMax;

        // find min and max values to stablish the range of the Y axis
        // however, if a new series brings more xtreme values, we need to respect that!
        qreal yMin = y.minCoeff();
        qreal yMax = y.maxCoeff();
        if (yMin < _yMin)
            _yMin = yMin;
        if (yMax > _yMax)
            _yMax = yMax;

#if (DEBUG > 1) && (DEBUG < 3)
        qDebug() << "plot(x,y): xrange [" << _xMin << "," << _xMax <<
                     "]  yrange [" << _yMin << "," << _yMax << "]";
#endif

        QtCharts::QXYSeries* series = NULL;
        if (marker == "o" || marker == "s") // it's a scatter plot!
        {
#if (DEBUG > 1) && (DEBUG < 3)
            qDebug() << "plot(x,y): scatter plot";
#endif
            QtCharts::QScatterSeries* s = new QtCharts::QScatterSeries();
            s->setMarkerSize(markersize); // symbol size

            if (marker == "o")
                s->setMarkerShape(QtCharts::QScatterSeries::MarkerShapeCircle);

            if (marker == "s")
                s->setMarkerShape(QtCharts::QScatterSeries::MarkerShapeRectangle);

            series = (QtCharts::QXYSeries*)s;
        }
        else // draw line
        {
#if (DEBUG > 1) && (DEBUG < 3)
            qDebug() << "plot(x,y): line plot";
#endif
            series = new QtCharts::QLineSeries();
        }

        // Call a string parser! Ex: "label=Trump Tweets" becomes "Trump Tweets"
        _parseLegend();
        if (_legend.size())
        {
#if (DEBUG > 1) && (DEBUG < 3)
            qDebug() << "plot(x,y): label=" << _legend;
#endif
            series->setName(_legend);
        }

        for (int i = 0; i < x.rows(); i++)
        {
#if (DEBUG > 1) && (DEBUG < 3)
            qDebug() << "plot(x,y): x[" << i << "]=" << x[i] << " y[" << i << "]=" << y[i];
#endif
            series->append(x[i], y[i]);
        }

        // Customize series color and transparency
        QColor fillColor = color;
        if (fillColor == DEFAULT_COLOR)
            fillColor = _colors[_colorIdx++];
        fillColor.setAlphaF(alpha);

        QPen pen = series->pen();
        pen.setWidth(linewidth);

        if (marker == "o" || marker == "s")
        {
            if (edgecolor == DEFAULT_EDGECOLOR)
            {
                pen.setColor(fillColor);        // outline should be invisible
#if (DEBUG > 1) && (DEBUG < 3)
                qDebug() << "plot(x,y): fillColor=" << fillColor;
#endif
            }
            else
            {
#if (DEBUG > 1) && (DEBUG < 3)
                qDebug() << "plot(x,y): edgecolor=" << edgecolor;
#endif
                QColor edgeColor = edgecolor;
                edgeColor.setAlphaF(alpha);
                pen.setColor(edgeColor);       // create circle outline
            }
        }
        else if (marker == "--")
        {
            pen.setStyle(Qt::DashLine);
            pen.setColor(fillColor);
        }
        else if (marker == ".")
        {
            pen.setStyle(Qt::DotLine);
            pen.setColor(fillColor);
        }
        else // marker == "-"
        {
            pen.setColor(fillColor);
        }

        series->setPen(pen);
        series->setBrush(QBrush(fillColor));

        if (_colorIdx >= _colors.size())
            _colorIdx = 0;

        _seriesVec.push_back(series);

#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "plot(x,y): -----";
#endif
    }    

    /* show(): displays all the data added through plot() calls.
     */
    void show()
    {        
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "show(): " << _title;
#endif
        if (!_seriesVec.size())
        {
            qCritical() << "show()!!! Must set the data with plot() before show().";
            return;
        }

        /* Customize chart title */

        QFont font;
        font.setPixelSize(12);
        font.setWeight(QFont::Bold);
        _chart->setTitleFont(font);
        _chart->setTitle(_title);

        //TODO: investigate detaching the legend for custom positioning
        //https://doc.qt.io/qt-5/qtcharts-legend-example.html

        if (_legendPos.size())
        {
            if (_legendPos == "lower center")      // 9
                _chart->legend()->setAlignment(Qt::AlignBottom);
            else if (_legendPos == "upper center") // 8
                _chart->legend()->setAlignment(Qt::AlignTop);
            else if (_legendPos == "center right") // 7
                _chart->legend()->setAlignment(Qt::AlignRight);
            else if (_legendPos == "center left") // 6
                _chart->legend()->setAlignment(Qt::AlignLeft);
            else
            {
                qCritical() << "show()!!!" << _legendPos << " is not a valid legend position.";
                _chart->legend()->setAlignment(Qt::AlignBottom);
            }
        }

        if (_legend.size())
            _chart->legend()->setVisible(true);
        else
            _chart->legend()->setVisible(false);

        /* Customize X, Y axis and categories */

#if (DEBUG > 1) && (DEBUG < 3)
        qDebug() << "show(): xrange [" << _xMin << "," << _xMax << "] " <<
                     " yrange [" << _yMin << "," << _yMax << "]";
#endif

        QPen axisPen(Qt::black); // default axis line color and width
        axisPen.setWidth(1);

        QtCharts::QValueAxis* axisX = NULL;
        QtCharts::QCategoryAxis* categoryX = NULL;
        if (_showXticks == SHOW_TICK || _showXticks == HIDE_TICK)
        {
            axisX = new QtCharts::QValueAxis;
            axisX->setGridLineVisible(_enableGrid);
            axisX->setTitleText(_xLabel);
            axisX->setLinePen(axisPen);
            axisX->setRange(_xMin, _xMax);
            axisX->setTickCount(_xTickCount);
            if (!_customLimits) axisX->applyNiceNumbers();
            _chart->addAxis(axisX, Qt::AlignBottom);

            if (_showXticks == HIDE_TICK)
                axisX->setLabelsVisible(false);
        }
        else if (_showXticks == SHOW_CUSTOM_TICK)
        {
            categoryX = new QtCharts::QCategoryAxis();
            categoryX->setGridLineVisible(_enableGrid);
            categoryX->setLinePen(axisPen);

            if (_showXticks && _xTicks.size())
                for (int i = 0; i < _xTicks.size(); i++)
                {
#if (DEBUG > 1) && (DEBUG < 3)
                    qDebug() << "show(): xtick[" << i << "]=(" << _xTicks[i].second << " , " <<
                                _xTicks[i].first << ")";
#endif
                    categoryX->append(_xTicks[i].first, _xTicks[i].second);
                }

            categoryX->setRange(_xMin, _xMax);
            categoryX->setTickCount(_xTicks.size());
            _chart->addAxis(categoryX, Qt::AlignBottom);
        }

        QtCharts::QValueAxis* axisY = NULL;
        QtCharts::QCategoryAxis* categoryY = NULL;
        if (_showYticks == SHOW_TICK || _showYticks == HIDE_TICK) // this is the default ticks setup
        {
            axisY = new QtCharts::QValueAxis;
            axisY->setGridLineVisible(_enableGrid);
            axisY->setTitleText(_yLabel);
            axisY->setLinePen(axisPen);
            axisY->setRange(_yMin, _yMax);
            axisY->setTickCount(_yTickCount);
            if (!_customLimits) axisY->applyNiceNumbers();
            //axisY->setBase(8.0); // 1, 8, 64, 512, 4096
            _chart->addAxis(axisY, Qt::AlignLeft);

            if (_showYticks == HIDE_TICK)
                axisY->setLabelsVisible(false);
        }
        else if (_showYticks == SHOW_CUSTOM_TICK) // this is for user defined ticks
        {
            categoryY = new QtCharts::QCategoryAxis();
            categoryY->setGridLineVisible(_enableGrid);
            categoryY->setLinePen(axisPen);

            if (_showYticks && _yTicks.size() > 0)
                for (int i = 0; i < _yTicks.size(); i++)
                {
#if (DEBUG > 1) && (DEBUG < 3)
                    qDebug() << "show(): ytick[" << i << "]=(" << _yTicks[i].second << " , " <<
                                _yTicks[i].first << ")";
#endif
                    categoryY->append(_yTicks[i].first, _yTicks[i].second);
                }

            categoryY->setRange(_yMin, _yMax);
            categoryY->setTickCount(_yTicks.size());
            _chart->addAxis(categoryY, Qt::AlignLeft);
        }

        /* Other possible customizations such as margins and background color */

        //_chart->setMargins(QMargins(0,0,0,0));
        //_chart->setPlotAreaBackgroundBrush(QBrush(Qt::black));
        //_chart->setPlotAreaBackgroundVisible(true);

        // Remove (fat) exterior margins from QChart
        _chart->layout()->setContentsMargins(0, 0, 0, 0);
        _chart->setBackgroundRoundness(0);

        /* Add series of data */

        for (int i = 0; i < _seriesVec.size(); i++)
        {
            _chart->addSeries(_seriesVec[i]);

            if (_showXticks == SHOW_TICK || _showXticks == HIDE_TICK)
            {
                _seriesVec[i]->attachAxis(axisX);
            }
            else if (_showXticks == SHOW_CUSTOM_TICK)
            {
                _seriesVec[i]->attachAxis(categoryX);
            }

            if (_showYticks == SHOW_TICK || _showYticks == HIDE_TICK)
            {
                _seriesVec[i]->attachAxis(axisY);
            }
            else if (_showYticks == SHOW_CUSTOM_TICK)
            {
                _seriesVec[i]->attachAxis(categoryY);
            }
        }

        _chartView->setRenderHint(QPainter::Antialiasing);
        _chartView->resize(600, 400);

        // Take a screenshot of the widget before it's destroyed
        // so it can be saved later, when savefig() is invoked after show().
        _pixmap = _chartView->grab();

        _chartView->show();

        // This loop blocks execution & waits for the window to be destroyed.
        // However, is this chart is supposed to be a real widget, then do none of this.
        if (!_isWidget)
        {
            _chartView->setAttribute(Qt::WA_DeleteOnClose); // This deletes _chartView!
            QEventLoop loop;
            QObject::connect(_chartView, SIGNAL(destroyed()), &loop, SLOT(quit()));
            loop.exec();

            // _chartView was automatically deleted. Release all the other resources!
            for (int i = 0; i < _seriesVec.size(); i++)
                delete _seriesVec[i];
        }

#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "show(): -----" ;
#endif
    }



private:

    bool _is_marker(const QString& cmd)
    {
        if (cmd == "-" || cmd == "--" || cmd == "." || cmd == "o" || cmd == "s")
            return true;

        return false; // cmd is a label for the legend
    }

    void _check_cmds_are_good(const QString& cmd1, const QString& cmd2)
    {
        if (_is_marker(cmd1) && !_is_marker(cmd2))
            return;

        if (!_is_marker(cmd1) && _is_marker(cmd2))
            return;

        qCritical() << "_check_cmds_are_good()!!! Only one marker and one label are allowed.";
        exit(-1);
    }

    void _parseLegend()
    {
        if (!_legend.size())
            return;

        QRegExp equalsRegex("(\\=)"); //RegEx for '='
        QStringList wordList = _legend.split(equalsRegex);

        // step1: trim spaces
        for (int i = 0; i < wordList.size(); i++)
            wordList[i] = wordList[i].trimmed();

        // step 2: delete empty strings
        QMutableStringListIterator it(wordList); // pass list as argument
        while (it.hasNext())
        {
            if (!it.next().size())
                it.remove();
        }

        int i = -1;
        QStringList::iterator iter = std::find(wordList.begin(), wordList.end(), "label");
        if (iter != wordList.end())
            i = iter - wordList.begin(); // if "loc" is in the list, i has the index

        // ok, "label" exists && there's another word after it on the list
        if (i >= 0 && i+1 < wordList.size())
            _legend = wordList[i+1];
        else
            _legend.clear();
    }

    QString _parseLegendPos(QString cmd)
    {
        if (!cmd.size())
            return QString();

        QRegExp equalsRegex("(\\=)"); //RegEx for '='
        QStringList wordList = cmd.split(equalsRegex);

        // step1: trim spaces
        for (int i = 0; i < wordList.size(); i++)
            wordList[i] = wordList[i].trimmed();

        // step 2: delete empty strings
        QMutableStringListIterator it(wordList); // pass list as argument
        while (it.hasNext())
        {
            if (!it.next().size())
                it.remove();
        }

        int i = -1;
        QStringList::iterator iter = std::find(wordList.begin(), wordList.end(), "loc");
        if (iter != wordList.end())
            i = iter - wordList.begin(); // if "loc" is in the list, i has the index

        // ok, "label" exists && there's another word after it on the list
        if (i >= 0 && i+1 < wordList.size())
            return wordList[i+1];

        return QString();
    }

    QPixmap _pixmap;                                // show() screenshots the widget, savefig() writes it on the disk
    QtCharts::QChart* _chart;                       // manages the graphical representation of the chart's series, legends & axes
    QtCharts::QChartView* _chartView;               // standalone widget that can display charts
    QVector<QtCharts::QXYSeries*> _seriesVec;      // every plot() creates a new series of data that is stored here

    bool _isWidget;                                // true: show() doesn't block so this can be used as widget
    QString _legend;
    QString _legendPos;

    QVector<QPair<QString, qreal> > _xTicks;       // user defined <label, endValue> ticks that replace default ticks
    QVector<QPair<QString, qreal> >  _yTicks;
    int _showXticks;                               // flag that show/hides the exhibition of ticks on the X axis
    int _showYticks;                               // flag that show/hides the exhibition of ticks on the Y axis
    int _xTickCount;                              // number of ticks displayed on the X axis
    int _yTickCount;                              // number of ticks displayed on the Y axis

    QString _title;                                 // Chart title
    QString _yLabel;                                // String that appears to the left of the Y axis
    QString _xLabel;                                // String that appears below the X axis

    QVector<QColor> _colors;                        // Vector that stores chart's predefined colours
    int _colorIdx;                                 // Every plot() increases this index so the next has a different colour

    bool _customLimits;                            // If the user has informed new limits through xlim(), ylim(), or axis()
    qreal _xMin;                                   // X axis min limit
    qreal _xMax;                                   // X axis max limit
    qreal _yMin;                                   // Y axis min limit
    qreal _yMax;                                   // Y axis max limit

    bool _enableGrid;                              // flag that show/hides the background grid
};
