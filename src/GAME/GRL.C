/* GAME/GRL.C - flat-framebuffer versions of the low-level draw routines. */
#include "../GR.H"

#include <stdlib.h>
#include <string.h>

static void draw_shape(vptype *vp, int x, int y, int width, int height,
                       const byte *shape, int table)
{
    int px, py;
    if (vp == NULL || shape == NULL || width <= 0 || height <= 0) return;
    for (py = 0; py < height; ++py) {
        for (px = 0; px < width; ++px) {
            uword mapped = cmtab[table & 3][shape[py * width + px]];
            if (mapped != 255) plot(vp, x + px, y + py, mapped);
        }
    }
}

void ldrawsh_cga(vptype *vp, int x, int y, int width_bytes, int height,
                 const byte *shape, int table)
{
    int px, py;
    for (py = 0; py < height; ++py) {
        for (px = 0; px < width_bytes * 4; ++px) {
            byte packed = shape[py * width_bytes + px / 4];
            byte source = (byte)((packed >> (6 - ((px & 3) << 1))) & 3);
            uword mapped = cmtab[table & 3][source];
            if (((mapped >> 8) & 3) != 0)
                plot(vp, x + px, y + py, mapped & 3);
        }
    }
}

void ldrawsh_ega(vptype *vp, int x, int y, int width_bytes, int height,
                 const byte *shape, int table)
{
    int px, py;
    int row_bytes;
    if (table != 3 || (x & 7) != 0) {
        row_bytes = (width_bytes + 1) / 2;
        for (py = 0; py < height; ++py) {
            for (px = 0; px < width_bytes; ++px) {
                byte packed = shape[py * row_bytes + px / 2];
                byte source = (byte)((packed >> ((px & 1) * 4)) & 15);
                uword mapped = cmtab[table & 3][source];
                if (mapped != 16) plot(vp, x + px, y + py, mapped);
            }
        }
    } else {
        row_bytes = width_bytes / 8;
        for (py = 0; py < height; ++py) {
            for (px = 0; px < row_bytes * 8; ++px) {
                int plane;
                byte source = 0;
                for (plane = 0; plane < 4; ++plane) {
                    byte packed = shape[plane * height * row_bytes +
                                        py * row_bytes + px / 8];
                    if ((packed & (0x80 >> (px & 7))) != 0)
                        source |= (byte)(8 >> plane);
                }
                {
                    uword mapped = cmtab[table & 3][source];
                    if (mapped != 16) plot(vp, x + px, y + py, mapped);
                }
            }
        }
    }
}

void ldrawsh_vga(vptype *vp, int x, int y, int width_bytes, int height,
                 const byte *shape, int table)
{
    int px, py;
    if (table != 3 || (x & 3) != 0) {
        draw_shape(vp, x, y, width_bytes, height, shape, table);
        return;
    }
    for (py = 0; py < height; ++py) {
        int plane_width = width_bytes / 4;
        for (px = 0; px < plane_width * 4; ++px) {
            int plane = px & 3;
            byte source = shape[(3 - plane) * height * plane_width +
                                py * plane_width + px / 4];
            uword mapped = cmtab[table & 3][source];
            if (mapped != 255) plot(vp, x + px, y + py, mapped);
        }
    }
}

void ldrawsh_mcga(vptype *vp, int x, int y, int width_bytes, int height,
                  const byte *shape, int table)
{
    ldrawsh_vga(vp, x, y, width_bytes, height, shape, table);
}

void scroll(vptype *vp, int x0, int y0, int x1, int y1, int xd, int yd)
{
    int width, height;
    int row;
    byte *page;
    byte *copy;
    if (vp == NULL) return;
    if (x0 < 0) x0 = 0;
    else if (x0 > vp->vpxl) x0 = vp->vpxl;
    if (x1 < 0) x1 = 0;
    else if (x1 > vp->vpxl) x1 = vp->vpxl;
    if (y0 < 0) y0 = 0;
    else if (y0 > vp->vpxl) y0 = vp->vpxl;
    if (y1 < 0) y1 = 0;
    else if (y1 > vp->vpxl) y1 = vp->vpxl;
    if (x0 >= x1 || y0 >= y1) return;

    if (xd > 0) x1 -= xd;
    else x0 -= xd;
    if (yd > 0) y1 -= yd;
    else y0 -= yd;

    width = x1 - x0;
    height = y1 - y0;
    if (width <= 0 || height <= 0) return;
    page = &jill_video[pagedraw != 0][0][0];
    copy = (byte *)malloc((size_t)width * height);
    if (copy == NULL) return;
    for (row = 0; row < height; ++row)
        memcpy(copy + (size_t)row * width,
               page + (vp->vpy + y0 + row) * JILL_SCREEN_WIDTH +
                      vp->vpx + x0,
               (size_t)width);
    for (row = 0; row < height; ++row)
        memcpy(page + (vp->vpy + y0 + yd + row) * JILL_SCREEN_WIDTH +
                      vp->vpx + x0 + xd,
               copy + (size_t)row * width,
               (size_t)width);
    free(copy);
}

void lcopypage(void)
{
    if ((x_ourmode & 0xfe) != x_ega &&
        (x_ourmode & 0xfe) != x_vga) return;
    memcpy(jill_video[pagedraw != 0], jill_video[pageshow != 0],
           sizeof(jill_video[0]));
}

void scrollvp(vptype *vp, int xd, int yd)
{
    scroll(vp, 0, 0, vp->vpxl, vp->vpyl, xd, yd);
}

void clrvp(vptype *vp, byte color)
{
    int x, y, width;

    switch (x_ourmode & 0xfe) {
    case x_cga:
        color = 0;
        break;
    case x_ega:
        color &= 15;
        break;
    case x_vga:
        break;
    default:
        return;
    }
    width = (vp->vpxl >> 3) << 3;
    for (y = 0; y < vp->vpyl; ++y)
        for (x = 0; x < width; ++x)
            plot(vp, x, y, color);
}
