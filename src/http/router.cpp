#include "hyperion/router.h"
#include "hyperion/file_cache.h"
#include "hyperion/static_file.h"
#include "hyperion/metrics.h"

namespace hyperion
{

    RouteResult Router::handle(const ParsedRequest &req)
    {

        // ── Bench — raw speed ──────────────────────────────────────────
        if (req.path == "/bench")
        {
            return RouteResult{
                200,
                "text/plain",
                "Hello from Hyperion"};
        }

        // ── Health check ───────────────────────────────────────────────
        if (req.path == "/health")
        {
            return RouteResult{
                200,
                "application/json",
                "{\"status\":\"ok\",\"server\":\"hyperion\"}"};
        }

        // ── Metrics ────────────────────────────────────────────────────
        if (req.path == "/metrics")
        {
            auto &m = Metrics::instance();

            std::string json;
            json += "{";
            json += "\"requests_total\":" + std::to_string(m.total_requests());
            json += ",\"requests_per_sec\":" + std::to_string(m.requests_per_second());
            json += ",\"active_connections\":" + std::to_string(m.active_connections());
            json += ",\"uptime_seconds\":" + std::to_string(m.uptime_seconds());
            json += ",\"threads\":4";
            json += "}";

            return RouteResult{200, "application/json", json};
        }

        // ── Root → index.html from CACHE ──────────────────────────────
        if (req.path == "/" || req.path == "/index.html")
        {
            const std::string *body =
                FileCache::instance().get("static/index.html");

            if (!body)
            {
                return RouteResult{
                    500, "text/plain", "Internal Server Error"};
            }

            return RouteResult{200, "text/html", *body};
        }

        // ── Other static files from cache ─────────────────────────────
        std::string file_path = "static" + req.path;
        const std::string *body = FileCache::instance().get(file_path);

        if (body)
        {
            return RouteResult{
                200,
                StaticFile::content_type(file_path),
                *body};
        }

        // ── 404 from cache ─────────────────────────────────────────────
        const std::string *not_found =
            FileCache::instance().get("static/404.html");

        return RouteResult{
            404,
            "text/html",
            not_found ? *not_found : "404 Not Found"};
    }

} // namespace hyperion