// OMStudio C API —— 供 C# 编辑器（P/Invoke）调用。
// 职责分离：C# 负责本体存储/属性面板/CRUD 业务，本 API 负责拓扑投影、拾取、
// 可见性收集；C# 侧只在变更时增量调用，不做每帧全量同步。
#pragma once

#ifdef _WIN32
  #ifdef OMSTUDIO_EXPORTS
    #define OM_API __declspec(dllexport)
  #else
    #define OM_API __declspec(dllimport)
  #endif
#else
  #define OM_API
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct om_entity {
    uint64_t id;           // 0 = 由引擎分配
    float* dims;           // 5~6 维语素向量（长度 = dim_count；输入可 const，输出可写）
    int dim_count;
    float radius;
    uint32_t color;        // 0xRRGGBB
    uint64_t parent_id;    // 0 = 根
    const char* label;     // 可为 NULL
} om_entity;

typedef struct om_render_node {
    uint64_t id;
    float x, y, z;
    float radius;
    uint32_t color;
    int lod;
} om_render_node;

typedef struct om_render_edge {
    float ax, ay, az, bx, by, bz;
    float strength;
} om_render_edge;

// ---- 生命周期与拓扑 ----
OM_API void om_init(const char* topology, int dim);   // topology: "torus" | "hyperbolic"
OM_API void om_shutdown(void);
OM_API void om_set_topology(const char* topology);    // 运行时切换构型
OM_API const char* om_topology_name(void);
OM_API void om_reproject(void);

// ---- 本体 CRUD ----
OM_API uint64_t om_add_entity(const om_entity* e);
OM_API int om_remove_entity(uint64_t id);
OM_API int om_get_entity(uint64_t id, om_entity* out); // out->dims 由调用方分配（dim_count 返回实际）
OM_API void om_update_entity(uint64_t id, const float* dims, int dim_count, float radius, uint32_t color);
OM_API uint64_t om_entity_count(void);
OM_API uint64_t om_selected(void);
OM_API void om_select(uint64_t id);                    // 选中并聚焦（拓扑中心重投影）
OM_API void om_clear_selection(void);

// ---- 关联 ----
OM_API void om_set_relation(uint64_t a, uint64_t b, float strength);
OM_API int om_remove_relation(uint64_t a, uint64_t b);

// ---- 相机与拾取（C# 编辑器端可直接用，无需依赖 3D 窗口）----
// NDC：x,y ∈ [-1,1]，y 向上；返回命中的实体 id（0 = 未命中）
OM_API void om_set_camera(float eyeX, float eyeY, float eyeZ,
                          float tgtX, float tgtY, float tgtZ,
                          float upX, float upY, float upZ,
                          float fovYDeg, float aspect, float zn, float zf);
OM_API uint64_t om_pick_screen(float ndcX, float ndcY);

// ---- 可见性收集（自绘/统计用；nodes/edges 缓冲由调用方分配，容量由 *Count 传入并回写）----
OM_API void om_collect_visible(float maxDist, float edgeStrengthThreshold,
                               om_render_node* nodes, int* nodeCount,
                               om_render_edge* edges, int* edgeCount);

#ifdef __cplusplus
}
#endif
