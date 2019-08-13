#include "httphelper.h"
#include "reglerapp.h"
#include "storage.h"
#include <sstream>


const char * const HttpHelper::SERVER_IP_STR     = "srv_ip";
const char * const HttpHelper::SERVER_PORT_STR   = "srv_port";
const char * const HttpHelper::SENSOR_ID_STR     = "srv_sensor_id";
const char * const HttpHelper::POINT_ID_STR      = "srv_point_id";

static const char* HTML_BEGIN = "<html>";
static const char* HTML_HEAD_STYLE = \
"<head>" \
    "<style>" \
        "body {" \
            "margin: 0px;" \
            "color: #cd7814;" \
            "background: black;" \
            "font-family: arial;" \
            "font-size: 14px;" \
        "}" \
        "input {" \
            "margin: 3px 0;" \
        "}" \
        ".statok {" \
            "display: inline-block;" \
            "color: #ffa200;" \
            "text-align: center;" \
        "}" \
        ".bar {" \
            "width: 100%;" \
            "position: absolute;" \
            "padding: 7px;" \
            "background: #fff3;" \
            "color: #eaeaea;" \
            "font-family: arial;" \
            "bottom: 0px;" \
        "}" \
        ".imgd {" \
            "position: relative;" \
            "text-align: center;" \
            "background: #272727;" \
            "padding: 20px 0 15px 0;" \
        "}" \
        ".main {" \
            "position: relative;" \
            "margin: 0px auto;" \
            "width: 160px;" \
        "}" \
        ".status {color: white; font-weight:bold}" \
        ".status_ok {color: lime; font-weight:bold}" \
        ".status_err {color: red; font-weight:bold}" \
    "</style>" \
"</head>";

static const char* HTML_BODY_BEGIN = "<body>" \
    "<div class=\"imgd\"><img src=\"data:image/png;base64,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\"></div><br>" \
    ;
static const char* HTML_DIV_MAIN_BEGIN = "<div class=\"main\">";
static const char* HTML_DIV_MAIN_END = "</div>";
static const char* HTML_END = "</body></html>";


HttpHelper::HttpHelper(ObjectType type, EspApp *app) :
    EspObject (type, app)
{
    m_reglerApp = static_cast<ReglerApp*>(app);
}

void HttpHelper::getResponseHtml(std::string &str)
{
    str.clear();
    str.reserve(3000);
    str.append(HTML_BEGIN);
    str.append(HTML_HEAD_STYLE);
    str.append(HTML_BODY_BEGIN);
    str.append(HTML_DIV_MAIN_BEGIN);
    str.append(m_reglerApp->responseHTML);
    str.append(HTML_DIV_MAIN_END);
    str.append(getStatisticHtml());
    str.append(HTML_END);
}

std::string HttpHelper::getStatisticHtml() {
    std::stringstream bar;
    if (m_reglerApp != nullptr) {
        bar << "<div class=\"bar\">  ";
        bar << "<div class=\"time\">" << getCurrentTimeString() << "</div>";
        bar << " &#x2591; STATUS: <div class=\"statok\">";
        bar << (m_reglerApp->statistic.active ? "OK" : "OFFLINE");
        bar << "</div>  &#x2591; IN:" << m_reglerApp->statistic.in
                   << " &#x2591; OUT:" << m_reglerApp->statistic.out
                   << " &#x2591; SENDED:" << m_reglerApp->statistic.sended
                   << " &#x2591; FAILED:<div class=\"statok\">" << m_reglerApp->statistic.failed_req
                   << "</div></div>";
    }
    return bar.str().c_str();
}

std::string HttpHelper::getCurrentTimeString()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timeinfo);
    return std::string(buffer);
}

bool HttpHelper::reset(std::string *response)
{
    Storage * storage = Storage::instance();
    if (storage) {
        m_newEvent = std::make_shared<Event>(INITIAL_TRANSITION, this);
        // clear wifi connection
        storage->write("pass", "");
        // clear server data info
        storage->write(SERVER_IP_STR, "");
        storage->write_int(SERVER_PORT_STR, 0);
        storage->write_int(POINT_ID_STR, 0);
        storage->write_int(SENSOR_ID_STR, 0);
    }
    *response = m_reglerApp->page1 + "Device reset OK." + m_reglerApp->page2;

    return true;
}

void HttpHelper::fillStatusResponseHtml()
{
    ReglerApp * app = static_cast<ReglerApp *> (m_app);
    std::string ip_string = app->m_local_ip.toString().c_str();


    Storage * storage= Storage::instance();
    std::string server_ip = storage->read(SERVER_IP_STR);
    int server_port  = storage->read_int(SERVER_PORT_STR);
    int sensor_id    = storage->read_int(SENSOR_ID_STR);
    int point_id     = storage->read_int(POINT_ID_STR);

    std::stringstream ss;
    ss << "Local IP: <span class=\"status\">" << ip_string << "</span><br><br>";
    ss << "IP: <span class=\"status\">" << server_ip << ":" << server_port << "</span><br>";
    ss << "Point id: <span class=\"status\">" << point_id << "</span><br>";
    ss << "Sensor id: <span class=\"status\">" << sensor_id << "</span><br><br>";

    if (m_reglerApp->statistic.active)
    {
        ss << "Status: <span class=\"status_ok\">OK</span><br>";
    }
    else
    {
        ss << "Status: <span class=\"status_err\">OFFLINE</span><br>";
        ss << "Err: <span class=\"status_err\">" << app->lastErrMsg << "</span><br>";
    }

    m_reglerApp->responseHTML = ss.str();
}
