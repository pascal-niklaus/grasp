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

// LeakSanitizer config. Suppresses leaks related to
// GTK3/Pango/fontconfig. These are not leaks in grasp, so ignore.
extern "C" const char *__lsan_default_suppressions() {
    return "leak:libqgtk3.so\n"
           "leak:libgtk-3.so\n"
           "leak:libgobject-2.0.so\n"
           "leak:libpango\n"
           "leak:libfontconfig.so\n"
           "leak:libharfbuzz.so\n"
           "leak:libexpat.so\n";
}
