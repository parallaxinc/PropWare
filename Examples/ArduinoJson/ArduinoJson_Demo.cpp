/**
 * @file    ArduinoJson_Demo.cpp
 *
 * @author  David Zemon
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2013 David Zemon<br>
 * <br>Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:<br>
 * <br>The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.<br>
 * <br>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <PropWare/gpio/pin.h>
#include <PropWare/hmi/output/printer.h>
#include <ArduinoJson.hpp>

using PropWare::Pin;
using PropWare::Port;
using ArduinoJson::StaticJsonBuffer;
using ArduinoJson::JsonObject;

int main () {
    char json[]                 = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";

    StaticJsonBuffer<200> jsonBuffer;
    const JsonObject      &root = jsonBuffer.parseObject(json);

    const char   *sensor   = root["sensor"];
    unsigned int time      = root["time"];
    double       latitude  = root["data"][0];
    double       longitude = root["data"][1];

    pwOut << "Sensor: " << sensor << "\n";
    pwOut << "Time: " << time << "\n";
    pwOut << "Latitude: " << latitude << "\n";
    pwOut << "Longitude: " << longitude << "\n";

    return 0;
}
