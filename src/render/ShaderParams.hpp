
typedef enum
    {
    /*----------------------------------------------------------
    Set 0 (constant whole frame)
    ----------------------------------------------------------*/
    VKN_SHADER_PARAM_SET_0_FIRST,
    /* begin */
    VKN_SHADER_PARAM_VECTOR_NAME_TIME = VKN_SHADER_PARAM_SET_0_FIRST,
    VKN_SHADER_PARAM_VECTOR_NAME_ONE_OVER_GAMMA,
    /* end */
    VKN_SHADER_PARAM_SET_0_LAST = VKN_SHADER_PARAM_VECTOR_NAME_ONE_OVER_GAMMA,

    /*----------------------------------------------------------
    Set 1 (constant per pass)
    ----------------------------------------------------------*/
    VKN_SHADER_PARAM_SET_1_FIRST,
    /* begin */
    VKN_SHADER_PARAM_VECTOR_NAME_PROJECTION_X = VKN_SHADER_PARAM_SET_1_FIRST,
    VKN_SHADER_PARAM_VECTOR_NAME_PROJECTION_Y,
    VKN_SHADER_PARAM_VECTOR_NAME_PROJECTION_Z,
    VKN_SHADER_PARAM_VECTOR_NAME_PROJECTION_W,

    VKN_SHADER_PARAM_VECTOR_NAME_DIFFUSE_LIGHT_DIR,
    /* end */
    VKN_SHADER_PARAM_SET_1_LAST = VKN_SHADER_PARAM_VECTOR_NAME_DIFFUSE_LIGHT_DIR,

    /*----------------------------------------------------------
    Set 2 (constant per object)
    ----------------------------------------------------------*/
    VKN_SHADER_PARAM_SET_2_FIRST,
    /* begin */
    VKN_SHADER_PARAM_VECTOR_NAME_VIEW_X = VKN_SHADER_PARAM_SET_2_FIRST,
    VKN_SHADER_PARAM_VECTOR_NAME_VIEW_Y,
    VKN_SHADER_PARAM_VECTOR_NAME_VIEW_Z,
    VKN_SHADER_PARAM_VECTOR_NAME_VIEW_W,

    VKN_SHADER_PARAM_VECTOR_NAME_MODEL_X,
    VKN_SHADER_PARAM_VECTOR_NAME_MODEL_Y,
    VKN_SHADER_PARAM_VECTOR_NAME_MODEL_Z,
    VKN_SHADER_PARAM_VECTOR_NAME_MODEL_W,

    VKN_SHADER_PARAM_VECTOR_NAME_MODELVIEW_X,
    VKN_SHADER_PARAM_VECTOR_NAME_MODELVIEW_Y,
    VKN_SHADER_PARAM_VECTOR_NAME_MODELVIEW_Z,
    VKN_SHADER_PARAM_VECTOR_NAME_MODELVIEW_W,

    VKN_SHADER_PARAM_VECTOR_NAME_VIEWPROJECTION_X,
    VKN_SHADER_PARAM_VECTOR_NAME_VIEWPROJECTION_Y,
    VKN_SHADER_PARAM_VECTOR_NAME_VIEWPROJECTION_Z,
    VKN_SHADER_PARAM_VECTOR_NAME_VIEWPROJECTION_W,

    VKN_SHADER_PARAM_VECTOR_NAME_MODELVIEWPROJECTION_X,
    VKN_SHADER_PARAM_VECTOR_NAME_MODELVIEWPROJECTION_Y,
    VKN_SHADER_PARAM_VECTOR_NAME_MODELVIEWPROJECTION_Z,
    VKN_SHADER_PARAM_VECTOR_NAME_MODELVIEWPROJECTION_W,

    VKN_SHADER_PARAM_VECTOR_NAME_COLOR_0,
    /* end */
    VKN_SHADER_PARAM_SET_2_LAST = VKN_SHADER_PARAM_VECTOR_NAME_COLOR_0,

    /*----------------------------------------------------------
    Set 3 (constant only per draw)
    ----------------------------------------------------------*/
    VKN_SHADER_PARAM_SET_3_FIRST,
    /* begin */
    /* end */
    VKN_SHADER_PARAM_SET_3_LAST = VKN_SHADER_PARAM_SET_3_FIRST - 1,

    /*----------------------------------------------------------
    Count
    ----------------------------------------------------------*/
    VKN_SHADER_PARAM_VECTOR_NAME_CNT
    } VKN_shader_param_vector_name_type;
