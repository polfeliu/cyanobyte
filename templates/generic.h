{% import 'macros.jinja2' as utils %}
{% import 'clang.jinja2' as cpp %}
{% import 'generic.jinja2' as embedded %}
{% set template = namespace(math=false) %}
/*
{{ utils.pad_string('* ', utils.license(info.copyright.name, info.copyright.date, info.license.name)) -}}
*
* Auto-generated file for {{ info.title }} v{{ info.version }}.
* Generated from {{ fileName }} using Cyanobyte Codegen v{{ version }}
* Class for {{ info.title }}
{{utils.pad_string("* ", info.description)}}
*/

#ifndef _{{info.title}}_H_
#define _{{info.title}}_H_

{# Create enums for functions #}
{% if functions %}
{% for key,function in functions|dictsort %}
{# Check if we need to import `math` lib #}
{% if 'computed' in function %}
{% for ckey,compute in function.computed|dictsort %}
{% macro scanForMathLib(logicKeys) -%}
{% for logic in logicKeys %}
{% if logic is mapping %}
{% for logicKey in logic.keys() %}
{% if logicKey == 'power' or logicKey == 'arc tangent' %}
{% if template.math is sameas false %}
#include <math.h>
{% set template.math = true %}
{% endif %}
{% elif logic[logicKey] is iterable and logic[logicKey] is not string -%}
{{- scanForMathLib(logic[logicKey]) -}}
{% endif %}
{% endfor %}
{% endif %}
{% endfor %}
{%- endmacro %}
{{- scanForMathLib(compute.logic) -}}
{% endfor %}
{% endif %}
{% endfor %}
{% endif %}

{# Create enums for fields #}
{% if fields %}
{% for key,field in fields|dictsort %}
{% if field.enum %}
{# Create enum #}
/*
{{utils.pad_string(" * ", "Valid values for " + field.title)}}
 */
enum {{key}} {
    {% set args = namespace(index=0) %}
    {% for ekey,enum in field.enum|dictsort %}
    {{key.upper()}}_{{ekey.upper()}} = {{enum.value}}{{- "," if args.index + 1 < field.enum | length }} // {{enum.title}}
    {% set args.index = args.index + 1 %}
    {% endfor %}
};
typedef enum {{key}} {{key}}_t;
{% endif %}
{% endfor %}
{% endif %}
{% if i2c.address is iterable and i2c.address is not string %}
enum deviceAddress {
    {% for address in i2c.address %}
    I2C_ADDRESS_{{address}} = {{address}}{{ "," if not loop.last }}
    {% endfor %}
};
typedef enum deviceAddress deviceAddress_t;
{% endif %}

{% if i2c.address is iterable and i2c.address is not string %}
int {{info.title.lower()}}_init(deviceAddress_t address, int (*connect)(uint8_t));
{% else %}
int {{info.title.lower()}}_init(int (*connect)(uint8_t));
{% endif %}
   
{% for key,register in registers|dictsort -%}
{% set length = (register.length / 8) | round(1, 'ceil') | int %}
{% if (not 'readWrite' in register) or ('readWrite' in register and 'R' is in(register.readWrite)) %}
/**
 {{utils.pad_string(" * ", register.description)}}
*/
int {{info.title.lower()}}_read{{key}}(
    {{cpp.numtype(register.length)}}* val,
    int (*read)(uint8_t, uint8_t, {{cpp.numtype(register.length)}}*, uint8_t)
);
{% endif %}

{% if (not 'readWrite' in register) or ('readWrite' in register and 'W' is in(register.readWrite)) %}
/**
{{utils.pad_string(" * ", register.description)}}
 */
int {{info.title.lower()}}_write{{key}}(
    {% if register.length > 0 %}
    {{cpp.numtype(register.length)}}* data,
    {% endif %}
    int (*read)(uint8_t, uint8_t, {{cpp.numtype(register.length)}}*, uint8_t),
    int (*write)(uint8_t, uint8_t, {{cpp.numtype(register.length)}}*, uint8_t)
);
{% endif %}
{%- endfor %}
{% if fields %}
{% for key,field in fields|dictsort %}
{% if 'R' is in(field.readWrite) %}
{% set int_t = cpp.registerSize(registers, field.register[12:]) %}
/**
{{utils.pad_string(" * ", field.description)}}
 */
int {{info.title.lower()}}_get_{{key.lower()}}(
    {{int_t}}* val,
    int (*read)(uint8_t, uint8_t, int*, uint8_t)
);
{% endif %}
{% if 'W' is in(field.readWrite) %}
{% set int_t = cpp.registerSize(registers, field.register[12:]) %}
/**
{{utils.pad_string(" * ", field.description)}}
 */
int {{info.title.lower()}}_set_{{key.lower()}}(
    {{int_t}}* data,
    int (*read)(uint8_t, uint8_t, int*, uint8_t),
    int (*write)(uint8_t, uint8_t, int*, uint8_t)
);
{% endif %}
{% endfor %}
{% endif %}

{% if functions %}
{% for key,function in functions|dictsort %}
{% for ckey,compute in function.computed|dictsort %}
/**
{{utils.pad_string(" * ", function.description)}}
*/
{# Look for any callbacks #}
{% set useCallback = namespace(delay=false) %}
{% if compute.logic %}
{% for stepk in compute.logic %}
{% for key,value in stepk|dictsort %}
{% if key == '$delay' %}
{# This won't work for nested delays #}
{% set useCallback.delay = value %}
{%- endif %}
{%- endfor %}
{%- endfor %}
{%- endif %}
{% if useCallback.delay %}
{# We need to define a struct for the callback #}
{% set int_t = cpp.returnType(compute) %}
struct {{useCallback.delay.name}}Callback {
    {{int_t}}* (*callback({{useCallback.delay.name}}Callback, float, *int, *int)) callback;
    // Include all functions -- A snapshot of function state
{{ cpp.variables(compute.variables) }}
}

{{useCallback.delay.name}}Callback {{info.title.lower()}}_{{key.lower()}}_{{ckey.lower()}}(
{{ embedded.functionParams(cpp, functions, compute, true) }}
{% else %}
void {{info.title.lower()}}_{{key.lower()}}_{{ckey.lower()}}(
{{ embedded.functionParams(cpp, functions, compute, false) }}
{% endif %}
);
{% endfor %}

{% endfor %}
{% endif %}

#endif