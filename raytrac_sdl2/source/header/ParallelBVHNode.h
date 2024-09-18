#pragma once
#include <algorithm>
#include <future>
#include <vector>
#include <memory>
#include <atomic>
#include "Hittable.h"
#include "AABB.h"
#include "OptixWrapper.h"



class ParallelBVHNode : public Hittable {
private:
    static constexpr size_t MIN_OBJECTS_PER_THREAD = 500;
    static std::atomic<int> active_threads;
    const int MIN_OBJECTS_PER_LEAF = 2;


public:

    ParallelBVHNode(const std::vector<std::shared_ptr<Hittable>>& src_objects,
        size_t start, size_t end, double time0, double time1, bool use_optix = false);
    bool isLeaf() const;
    std::shared_ptr<Hittable> left;
    std::shared_ptr<Hittable> right;
    bool bounding_box(double time0, double time1, AABB& output_box) const;
    bool use_optix;
   

     bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const ;

private:
    static bool box_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b, int axis);
    static bool box_x_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b);
    static bool box_y_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b);
    static bool box_z_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b);

    AABB box;
    std::shared_ptr<OptixWrapper> optix_wrapper;
};