
#include "sys_includes.h"
#include "global.h"
#include "debug.h"
#include "config.h"

#include "httpjson.h"
#include "darknetif.h"
#include "base64.h"

#include "darknet.h"

list *options;
char *name_list;
char **names;

image **alphabet;
network *net;

char buff[256];
char *input = buff;
float nms=.45;

void my_save_image_jpg(image im, const char *name)
{
    unsigned char *data = calloc(im.w*im.h*im.c, sizeof(char));
    int i,k;
    for(k = 0; k < im.c; ++k){
        for(i = 0; i < im.w*im.h; ++i){
            data[i*im.c+k] = (unsigned char) (255*im.data[i + k*im.w*im.h]);
        }
    }
    extern int stbi_write_jpg(char const *filename, int w, int h, int comp, const void *data, int quality);
    int success = stbi_write_jpg(name, im.w, im.h, im.c, data, 80);
    free(data);
    if(!success) fprintf(stderr, "Failed to write image %s\n", name);
}


int darknetif_init(void)
{
    names = get_labels(config.labelfile);
    alphabet = load_alphabet();
    net = load_network(config.cfgfile, config.weightfile, 0);
    set_batch_network(net, 1);
    srand(2222222);
    return 0;
}

int darknetif_process(struct httpjson_job *job, struct httpjson_job_reply *reply)
{
    //so, instead of hacking a new image loader, I'm simply going to write to /tmp/darknet_rest.in.jpg and to /tmp/darknet_rest.out.jpg
    //This makes this processor a singleton!
    static char bin_image[CONFIG_POST_DATA_SIZE_MAX+1];
    static char mark_image[CONFIG_POST_DATA_SIZE_MAX+1];
    static char *infile = "/tmp/darknet_rest.in.jpg";
    static char *outfile = "/tmp/darknet_rest.out.jpg";
    int i;
    i = my_b64_pton(job->image, (unsigned char*)bin_image, sizeof(bin_image));
    DBG3("Base64 decode returned size %d\n", i);
    if (i < 1) {
        WRN("base64 decode is less than 1, invalid base64 blob\n");
        reply->error = 1;
        reply->description = "base64 decode is less than 1, invalid base64 blob";
        return -1;
    }

    FILE *fp;
    fp = fopen(infile, "w");
    fwrite(bin_image, 1, i, fp);
    fclose(fp);

    image im = load_image_color(infile,0,0); //input = fname
    if (im.data == NULL) {
        WRN("could not load image\n");
        reply->error = 1;
        reply->description = "could not load image";
        return -1;        
    }

    image sized = letterbox_image(im, net->w, net->h);
    //image sized = resize_image(im, net->w, net->h);
    //image sized2 = resize_max(im, net->w);
    //image sized = crop_image(sized2, -((net->w - sized2.w)/2), -((net->h - sized2.h)/2), net->w, net->h);
    //resize_network(net, sized.w, sized.h);
    layer l = net->layers[net->n-1];

    float *X = sized.data;
    double mytime;
    mytime=what_time_is_it_now();
    network_predict(net, X);
    snprintf(reply->predict_time, sizeof(reply->predict_time), "%0.1f", what_time_is_it_now()-mytime);
    printf("%s: Predicted in %s seconds.\n", infile, reply->predict_time);
    int nboxes = 0;
    detection *dets = get_network_boxes(net, im.w, im.h, config.thresh, config.hier_thresh, 0, 1, &nboxes);
    //printf("%d\n", nboxes);
    //if (nms) do_nms_obj(boxes, probs, l.w*l.h*l.n, l.classes, nms);
    if (nms) do_nms_sort(dets, nboxes, l.classes, nms);
    draw_detections(im, dets, nboxes, config.thresh, names, alphabet, l.classes);

    {
        int i,j;
        for(i = 0; i < nboxes; ++i) {
            for(j = 0; j < l.classes; ++j) {
                if (dets[i].prob[j] > config.thresh) {
                    if (0 == strcmp("person", names[j])) {
                        reply->is_person = 1;
                        reply->is_person_confidence = (int)(names[j], dets[i].prob[j]*100);
                        INF("Found a Person! Confidence is %d %%\n", reply->is_person_confidence);
                    }
                }
            }
        }
    }

    free_detections(dets, nboxes);
    
    //extern void save_image_jpg(image p, const char *name);    
    //save_image(im, outfile_noext);
    my_save_image_jpg(im, outfile);

    free_image(im);
    free_image(sized);

    //fetch outfile and base64 encode to return data
    DBG3("fopen [%s]\n", outfile);
    fp = fopen(outfile, "r");
    DBG3("fread up to %d bytes\n", (int)sizeof(mark_image));
    i = fread((char*)mark_image, 1, sizeof(mark_image), fp);
    DBG3("fread returned %d\n", i);
    fclose(fp);

    if (job->return_marked_image) {
        DBG3("b64_ntop encoding %d bytes\n", i);
        i = my_b64_ntop((unsigned char*)mark_image, i, reply->mark_image, sizeof(reply->mark_image));
        DBG3("b64_ntop returned %d\n", i);
    }
    if (job->return_original_image) {
        //strncpy(reply->org_image, job->image, sizeof(reply->org_image));
    }
    
    INF("Prediction done. Returning result\n");
    return 0;
}
