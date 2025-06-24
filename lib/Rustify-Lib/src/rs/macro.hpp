#pragma once

#define return_case(__v) case __v: return #__v;
#define return_default() default: return "Invalid";