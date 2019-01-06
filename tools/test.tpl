{%- macro print_value(value) -%}
{%- if value is none -%}
NULL
{%- elif not value is number -%}
"{{ value }}"
{%- else -%}
{{ value }}
{%- endif -%}
{%- endmacro -%}

{%- macro print_conditions(conds) -%}
{%- for cond in conds -%}
{%- if loop.first %} WHERE{%- else %} AND
{%- endif %} {{ cond['name'] }}{{ cond['comp_op'] }}{{ print_value(cond['pivot']) }}
{%- endfor -%}
{%- endmacro -%}

{% for command in commands -%}
{%- if command['command'] == 'create database' -%}
CREATE DATABASE {{ command['database'] }};

{%- elif command['command'] == 'drop database' -%}
DROP DATABASE {{ command['database'] }};

{%- elif command['command'] == 'use' -%}
USE {{ command['database'] }};

{%- elif command['command'] == 'create table' -%}
CREATE TABLE {{ command['table'][1] }} (
    {%- for table_info in command['table'][0] %}
    {{ table_info['name'] }} {{ table_info['type'] }}
    {%- if table_info['type'] == 'CHAR' or table_info['type'] == 'VARCHAR' -%}
    ({{ table_info['len'] }})
    {%- endif -%}
    {%- if not loop.last -%},{%- endif -%}
    {%- endfor %}
);

{%- elif command['command'] == 'insert' -%}
INSERT INTO {{ command['table'] }} VALUES (
{%- for value in command['values'] -%}
{{ print_value(value) }}
{%- if not loop.last -%},{%- endif -%}
{%- endfor -%});

{%- elif command['command'] == 'delete' -%}
DELETE FROM {{ command['table'] }}{{ print_conditions(command['conditions']) }};

{%- elif command['command'] == 'update' -%}
UPDATE {{ command['table'] }} SET {{ command['update_column']}}={{ print_value(command['update_value']) }}{{ print_conditions(command['conditions']) }};

{%- elif command['command'] == 'select count' -%}
SELECT COUNT(*) FROM {{ command['table'] }}{{ print_conditions(command['conditions']) }};

{%- endif %}
{% endfor %}
