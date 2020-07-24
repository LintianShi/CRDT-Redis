//
// Created by user on 18-11-15.
//
#include "rpq_log.h"

void rpq_log::shift_up(int s)
{
    int j = s, i = (j - 1) / 2;
    auto temp = heap[j];
    while (j > 0)
    {
        if (*heap[i] >= *temp)
            break;
        else
        {
            heap[j] = heap[i];
            heap[j]->index = j;
            j = i;
            i = (i - 1) / 2;
        }
    }
    temp->index = j;
    heap[j] = temp;
}

void rpq_log::shift_down(int s)
{
    int i = s, j = 2 * i + 1, tail = static_cast<int>(heap.size() - 1);
    auto temp = heap[i];
    while (j <= tail)
    {
        if (j < tail && *heap[j] <= *heap[j + 1]) j++;
        if (*temp >= *heap[j])
            break;
        else
        {
            heap[i] = heap[j];
            heap[i]->index = i;
            i = j;
            j = i * 2 + 1;
        }
    }
    temp->index = i;
    heap[i] = temp;
}

void rpq_log::add(int k, double v)
{
    lock_guard<mutex> lk(mtx);
    if (map.find(k) == map.end())
    {
        map[k] = make_unique<element>(k, v);
        heap.emplace_back(map[k].get());
        shift_up(static_cast<int>(heap.size() - 1));
    }
    write_op_executed++;
}

void rpq_log::inc(int k, double i)
{
    if (i == 0) return;
    lock_guard<mutex> lk(mtx);
    if (map.find(k) != map.end())
    {
        auto &e = map[k];
        e->value += i;
        if (i > 0)
            shift_up(e->index);
        else
            shift_down(e->index);
    }
    write_op_executed++;
}

void rpq_log::rem(int k)
{
    lock_guard<mutex> lk(mtx);
    if (map.find(k) != map.end())
    {
        int i = map[k]->index;
        map.erase(k);
        heap[i] = heap.back();
        heap.erase(heap.end() - 1);
        shift_down(i);
    }
    write_op_executed++;
}

void rpq_log::max(int k, double v)
{
    int ak;
    double av;

    {
        lock_guard<mutex> lk(mtx);
        if (!heap.empty())
        {
            ak = heap[0]->name;
            av = heap[0]->value;
        }
        else
        {
            ak = -1;
            av = -1;
        }
    }

    {
        lock_guard<mutex> lk(max_mtx);
        max_log.emplace_back(k, v, ak, av);
    }
}

void rpq_log::overhead(int o)
{
    int num;
    {
        lock_guard<mutex> lk(mtx);
        num = heap.size();
    }
    lock_guard<mutex> lk(ovhd_mtx);
    overhead_log.emplace_back(num, o);
}

void rpq_log::write_logfiles()
{
    write_one_logfile("ovhd.csv", overhead_log);
    write_one_logfile("max.csv", max_log);
}

int rpq_log::random_get()
{
    lock_guard<mutex> lk(mtx);
    int r = -1;
    if (!heap.empty()) r = heap[intRand(static_cast<const int>(heap.size()))]->name;
    return r;
}
