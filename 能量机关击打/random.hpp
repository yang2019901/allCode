/* random.hpp
 * 用于提供产生随机数的头文件
 */

/* 辅助产生随机数的库 */
#include <ctime>
#include <sys/timeb.h>
#include <random>

#ifndef RANDOM_H
#define RANDOM_H

static timeb time_buffer;
static std::default_random_engine generator;

/* sample from normal distribution */
const double random_normal(const double mean, const double variance);

/* sample from uniform distribution */
const double random_uniform(const double mean, const double range);

#endif






