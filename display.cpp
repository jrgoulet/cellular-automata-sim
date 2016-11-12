//
// Created by Joe Goulet on 11/11/16.
//

#include "defs.h"
#include "State.h"
#include "Simulator.h"
#include <mpi.h>
#include <ncurses.h>

using namespace std;

/* Method declarations */
void display_row(int thread, int row, int width, vector<Node*>* nodev);
ostream& operator << (ostream& o, const Simulator& s);
string& operator += (string& s, const Simulator& n);
ostream& operator << (ostream& o, const State& s);
string& operator += (string& s, const State& n);

/* Constants */
#define INFO_W 10
#define INFO_H 2


/**
 * Initialize curses window
 */
void State::init_window() {
    int w = _width + INFO_W;
    int h = _height + INFO_H;
    _win_width = w;
    _win_height = h;
    if (_rank == 0) {
        initscr(); /* start curses */
        start_color(); /* start color mode */
        curs_set(0); /* hide cursor */
        resizeterm(h, w); /* resize curses window */
        scrollok(stdscr, FALSE); /* disallow scroll */
        nonl(); /* disallow new lines */
    }
}

/**
 * Resize curses window
 */
void State::adjust_window_width(int w) {
    if (w > _win_width) {
        _win_width = w;
    }
    if (_rank == 0) {
        resizeterm(_win_height, _win_width);
    }
}

/**
 * Displays a live view or snapshot of the current generation, using MPI_Barrier to
 * ensure that no threads are expecting stray messages. All threads send to master,
 * where the data is displayed.
 * @param delay
 */
void State::display_map(int delay) {
    MPI_Barrier(MPI_COMM_WORLD);
    string out = "";
    if (_rank != 0) {   /* slave : send map */
        for (int i = 0; i < _nodes->size(); i++) {
            int send[_width*2];
            for (int j = 0; j < _width; j++) send[j] = get_node_status(i, j);
            for (int j = _width; j < _width*2; j++) send[j] = get_node_color(i, j - _width);
            MPI_Send(&send,_width*2,MPI_INT,0,0,MPI_COMM_WORLD);
        }
    } else {    /* master : receive and display */
        int row = 1;

        /* Overwrite tiles with spaces */
        erase();

        /* Row 0 */
        out += *this;
        adjust_window_width((int) out.length());
        mvwaddstr(stdscr,0,0,out.c_str());

        /* Master thread row display */
        for (int j = 0; j < _nodes->size(); j++) {
            display_row(0,row,_width, get_row(j)->get_nodev());
            row++;
        }

        /* Slave thread row display */
        for (int j = 1; j < _height - _nodes->size(); j++) {
            tuple<int,int> bounds = get_bounds(_size,j,_height);
            int k = get<1>(bounds) - get<0>(bounds);
            for (int l = 0; l < k; l++) {
                int recv[_width*2];
                MPI_Status status;
                MPI_Recv(&recv,_width*2,MPI_INT,j,0,MPI_COMM_WORLD,&status);
                Row* r = new Row(recv,_width);
                display_row(j,row,_width,r->get_nodev());
                row++;
            }
        }

        /* Update tiles */
        refresh();
    }

    /* delay for visibility, 4000000 ns = 25 FPS max */
    timespec t0, t1;
    t0.tv_sec = 0;
    t0.tv_nsec = delay;
    nanosleep(&t0,&t1);
}


/**
 * Display Simulator
 */
void Simulator::display() {
    cout << _name << " | ";
    for (var v : *_ctrlv) cout << get<1>(v) << ": " << get<0>(v) << " | ";
    cout << "States: ";
    for (int i = 0; i < _langv->size(); i++) cout << "[" << i << "," << _langv->at((unsigned long) i) << "] ";
    cout << endl;
}

/**
 * Body of simulation screen
 * @param thread origin rank of thread containing nodes to be printed (for display)
 * @param row overall row number (for display)
 * @param intv pointer to a vector containing node values
 */
void display_row(int thread, int row, int width, vector<Node*>* nodev) {
    string prefix = ((row > 9) ? to_string(row) : ("0" + to_string(row))) + "|";
    int offset = (int) prefix.length();
    mvaddstr(row,0,prefix.c_str());
    for (int i = 0; i < nodev->size(); i++) { nodev->at((unsigned long) i)->display(row, i + offset); }
    string suffix = "|T"+((thread > 9) ? to_string(thread) : ("0" + to_string(thread)));
    mvaddstr(row,offset+width,(suffix.c_str()));
}

string print_row(int thread, int row, int width, vector<Node*>* nodev) {
    string out = "";
    out+= ((row > 9) ? to_string(row) : ("0" + to_string(row))) + "|";
    for (int i = 0; i < nodev->size(); i++) {
        out += Simulator::instance()->translate(nodev->at((unsigned long) i)->status());
    }
    out += "|T"+((thread > 9) ? to_string(thread) : ("0" + to_string(thread)));
    return out;
}

/**
 * Display exit message
 */
void State::display_exit() {
    curs_set(1);
    string msg = "Simulation Complete! Press [Enter] to continue.";
    int length = (int) msg.length();
    attron(COLOR_PAIR(1));
    mvaddstr(_height+1,(_width+10)/2-length/2,msg.c_str());
    attroff(COLOR_PAIR(1));
    refresh();
    getch();
    getch();
    endwin();
}

/**
 * Display individual node
 *
 * @param row Display window row
 * @param col Display window column
 */
void Node::display(int row, int col) {
    attron(COLOR_PAIR(_color));
    mvaddch(row,col,Simulator::instance()->translate(_state));
    attroff(COLOR_PAIR(_color));
}


/*
 * Operators ===================
 * Fun with operator overloading
 */


/**
 * Simulator <<
 * @param o
 * @param s
 * @return
 */
ostream& operator << (ostream& o, const Simulator& s) {
    for (int i = 0; i < 30; i++) o << "-"; o << endl;
    o << s._name << " (Mode " << s._mode << ")" << endl;
    o << "Vars" << "\t" << get<1>(s._ctrlv->at(0)) << ": " << get<0>(s._ctrlv->at(0)) << endl;
    for(ctrlv::iterator i = s._ctrlv->begin()+1; i != s._ctrlv->end(); i++) {
        o << "    \t" << get<1>(*i) << ": " << get<0>(*i) << endl;
    }
    o << "Map\t";
    for (int i = 0; i < s._langv->size(); i++) o << i << " ";
    o << endl << "   \t";
    for (char c : *s._langv) o << c << " ";
    o << endl;
    for (int i = 0; i < 30; i++) o << "-"; o << endl;
    return o;
}

/**
 * Simulator +=
 * @param s
 * @param n
 * @return
 */
string& operator += (string& s, const Simulator& n) {
    string config = "";
    config += n._name + " | ";
    for (var v : *n._ctrlv) {
        config += get<1>(v);
        config += ": ";
        config += to_string(get<0>(v));
        config += " | ";
    }
    config += "States: ";
    for (int i = 0; i < n._langv->size(); i++) {
        config += to_string(i);
        config += ":[";
        config += n._langv->at((unsigned long) i);
        config += "] ";
    }
    return s += config;
}

/**
 * State <<
 * @param o
 * @param s
 * @return
 */
ostream& operator << (ostream& o, const State& s) {
    o << s._rank << "| "<< "Mode:   " << s._mode    << "\t\t(" << s._height << "," << s._width << ")" << endl;
    o << s._rank << "| "<< "N:      " << s._current << " / "        << s._generations << endl;
    o << s._rank << "| "<< "Start:  " << s._start   << "\tTop:    " << s._top << "\tIgnition:  " << s._ignition << endl;
    o << s._rank << "| "<< "End:    " << s._end     << "\tBot:    " << s._bot << "\tGrowth:    " << s._growth << endl;
    for (int i = 0; i < s._nodes->size(); i++) {
        o << s._rank << "|  Trees:    \t[ "; for (int j : *s._nodes->at((unsigned long) i)->get_intv()) o << Simulator::instance()->translate(j); o << "]" << endl;
    }
    return o;
}

/**
 * State +=
 * @param s
 * @param n
 * @return
 */
string& operator += (string& s, const State& n) {
    string config = "G: ";
    config += to_string(n._current);
    config += " ";
    config += *Simulator::instance();
    return s += config;
}