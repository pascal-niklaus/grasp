/*
 * Grasp -- a program to GRAb the screen and Sketch on top during Presentations
 *
 * Copyright (C) 2024 Pascal Niklaus <pascal@niklaus.org>
 *
 * This file is part of 'Grasp'.
 *
 * 'Grasp' is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 'Grasp' is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 'Grasp'. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef VERBOSITY_H
#define VERBOSITY_H

// Verbosity
//
// Global singleton to access verbosity setting
//
class Verbosity {
  public:
    static int get();
    static void set(int verbosity);

  private:
    Verbosity() {}

    static Verbosity& get_instance();
    int _verbosity = 0;

  public:
    Verbosity(Verbosity const&) = delete;
    void operator=(Verbosity const&) = delete;
};

#endif
